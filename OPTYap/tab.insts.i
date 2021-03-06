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

/************************************************************************
**               Tabling instructions: auxiliary macros                **
************************************************************************/

#ifdef LOW_LEVEL_TRACER
#define store_low_level_trace_info(CP, TAB_ENT)  \
        CP->cp_pred_entry = TabEnt_pe(TAB_ENT)
#else
#define store_low_level_trace_info(CP, TAB_ENT)
#endif /* LOW_LEVEL_TRACER */

#define TABLING_ERROR_CHECKING_STACK                                             \
        TABLING_ERROR_CHECKING(store_node, Unsigned(H) + 1024 > Unsigned(B));    \
	TABLING_ERROR_CHECKING(store_node, Unsigned(H_FZ) + 1024 > Unsigned(B))


#define store_generator_node(TAB_ENT, SG_FR, ARITY, AP)                   \
        { register CELL *pt_args;                                         \
          register choiceptr gcp;                                         \
          /* store args */                                                \
          pt_args = XREGS + (ARITY);                                      \
	  while (pt_args > XREGS) {                                       \
            register CELL aux_arg = pt_args[0];                           \
            --YENV;                                                       \
            --pt_args;                                                    \
            *YENV = aux_arg;                                              \
	  }                                                               \
          /* initialize gcp and adjust subgoal frame field */             \
          YENV = (CELL *) (GEN_CP(YENV) - 1);                             \
          gcp = NORM_CP(YENV);                                            \
          SgFr_gen_cp(SG_FR) = gcp;                                       \
          /* store generator choice point */                              \
          HBREG = H;                                                      \
          store_yaam_reg_cpdepth(gcp);                                    \
          gcp->cp_tr = TR;                                                \
          gcp->cp_ap = (yamop *)(AP);                                     \
          gcp->cp_h  = H;                                                 \
          gcp->cp_b  = B;                                                 \
          gcp->cp_env = ENV;                                              \
          gcp->cp_cp = CPREG;                                             \
	  if (IsMode_Local(TabEnt_mode(TAB_ENT))) {                       \
            /* go local */                                                \
            register dep_fr_ptr new_dep_fr;                               \
            /* adjust freeze registers */                                 \
            H_FZ = H;                                                     \
            B_FZ = gcp;                                                   \
            TR_FZ = TR;                                                   \
            /* store dependency frame */                                  \
            new_dependency_frame(new_dep_fr, TRUE, LOCAL_top_or_fr, gcp,  \
                                 gcp, SG_FR, FALSE, LOCAL_top_dep_fr);    \
            LOCAL_top_dep_fr = new_dep_fr;                                \
            GEN_CP(gcp)->cp_dep_fr = LOCAL_top_dep_fr;                    \
          } else {                                                        \
            /* go batched */                                              \
            GEN_CP(gcp)->cp_dep_fr = NULL;                                \
          }                                                               \
          GEN_CP(gcp)->cp_sg_fr = SG_FR;                                  \
          store_low_level_trace_info(GEN_CP(gcp), TAB_ENT);               \
          set_cut((CELL *)gcp, B);                                        \
          B = gcp;                                                        \
          YAPOR_SET_LOAD(B);                                              \
          SET_BB(B);                                                      \
          TABLING_ERROR_CHECKING_STACK;                                   \
        }


#ifdef DETERMINISTIC_TABLING
#define store_deterministic_generator_node(TAB_ENT, SG_FR)            \
        { register choiceptr gcp;                                     \
          /* initialize gcp and adjust subgoal frame field */         \
          YENV = (CELL *) (DET_GEN_CP(YENV) - 1);                     \
	  gcp = NORM_CP(YENV);                                        \
          SgFr_gen_cp(SG_FR) = gcp;                                   \
          /* store deterministic generator choice point */            \
          HBREG = H;                                                  \
          store_yaam_reg_cpdepth(gcp);                                \
          gcp->cp_ap = COMPLETION;                                    \
          gcp->cp_b  = B;                                             \
          gcp->cp_tr = TR;           	  	                      \
          gcp->cp_h = H;                                              \
	  DET_GEN_CP(gcp)->cp_sg_fr = SG_FR;                          \
          store_low_level_trace_info(DET_GEN_CP(gcp), TAB_ENT);       \
          set_cut((CELL *)gcp, B);                                    \
          B = gcp;                                                    \
          YAPOR_SET_LOAD(B);                                          \
          SET_BB(B);                                                  \
          TABLING_ERROR_CHECKING_STACK;                               \
	}
#endif /* DETERMINISTIC_TABLING */




#define restore_generator_node(ARITY, AP)               \
        { register CELL *pt_args, *x_args;              \
          register choiceptr gcp = B;                   \
          /* restore generator choice point */          \
          H = HBREG = PROTECT_FROZEN_H(gcp);            \
          restore_yaam_reg_cpdepth(gcp);                \
          CPREG = gcp->cp_cp;                           \
          ENV = gcp->cp_env;                            \
          YAPOR_update_alternative(PREG, (yamop *) AP)  \
          gcp->cp_ap = (yamop *) AP;                    \
          /* restore args */                            \
          pt_args = (CELL *)(GEN_CP(gcp) + 1) + ARITY;  \
          x_args = XREGS + 1 + ARITY;                   \
          while (x_args > XREGS + 1) {                  \
            register CELL x = pt_args[-1];              \
            --x_args;                                   \
            --pt_args;                                  \
            *x_args = x;                                \
	  }                                             \
        }


#define pop_generator_node(ARITY)               \
        { register CELL *pt_args, *x_args;      \
          register choiceptr gcp = B;           \
          /* pop generator choice point */      \
          H = PROTECT_FROZEN_H(gcp);            \
          pop_yaam_reg_cpdepth(gcp);            \
          CPREG = gcp->cp_cp;                   \
          ENV = gcp->cp_env;                    \
          TR = gcp->cp_tr;                      \
          B = gcp->cp_b;                        \
          HBREG = PROTECT_FROZEN_H(B);		\
          /* pop args */                        \
          x_args = XREGS + 1 ;                  \
          pt_args = (CELL *)(GEN_CP(gcp) + 1);  \
	  while (x_args < XREGS + 1 + ARITY) {  \
            register CELL x = pt_args[0];       \
            pt_args++;                          \
            x_args++;                           \
            x_args[-1] = x;                     \
          }                                     \
          YENV = pt_args;		    	\
          SET_BB(PROTECT_FROZEN_B(B));          \
        }


#define store_consumer_node(TAB_ENT, SG_FR, LEADER_CP, DEP_ON_STACK)             \
        { register choiceptr ccp;                                                \
          register dep_fr_ptr new_dep_fr;                                        \
	  /* initialize ccp */                                                   \
          YENV = (CELL *) (CONS_CP(YENV) - 1);                                   \
          ccp = NORM_CP(YENV);                                                   \
          /* adjust freeze registers */                                          \
          H_FZ = H;                                                              \
          B_FZ = ccp;                    	                                 \
          TR_FZ = TR;                                                            \
          /* store dependency frame */                                           \
          new_dependency_frame(new_dep_fr, DEP_ON_STACK, LOCAL_top_or_fr,        \
                               LEADER_CP, ccp, SG_FR, FALSE, LOCAL_top_dep_fr);  \
          LOCAL_top_dep_fr = new_dep_fr;                                         \
          /* store consumer choice point */                                      \
          HBREG = H;                                                             \
          store_yaam_reg_cpdepth(ccp);                                           \
          ccp->cp_tr = TR;         	                                         \
          ccp->cp_ap = ANSWER_RESOLUTION;                                        \
          ccp->cp_h  = H;                                                        \
          ccp->cp_b  = B;                                                        \
          ccp->cp_env= ENV;                                                      \
          ccp->cp_cp = CPREG;                                                    \
          CONS_CP(ccp)->cp_dep_fr = LOCAL_top_dep_fr;                            \
          store_low_level_trace_info(CONS_CP(ccp), TAB_ENT);                     \
          /* set_cut((CELL *)ccp, B); --> no effect */                           \
          B = ccp;                                                               \
          YAPOR_SET_LOAD(B);                                                     \
          SET_BB(B);                                                             \
          TABLING_ERROR_CHECKING_STACK;                                          \
        }


#ifdef THREADS_FULL_SHARING_FTNA_3
#define FTNA_3_load_answer(ANSWER, SUBS_PTR)	        \
  /* Deref ANSWER */					\
  ans_node_ptr leaf_node = TrNode_entry(ANSWER);	\
  load_answer(leaf_node, subs_ptr PASS_REGS)
#else
#define FTNA_3_load_answer(ANSWER, SUBS_PTR)		\
  load_answer(ANSWER, SUBS_PTR PASS_REGS)
#endif /* THREADS_FULL_SHARING_FTNA_3 */

#ifdef THREADS_NO_SUBGOAL_TRIE_MIN_MAX

#define consume_answer_and_procceed_no_trie(DEP_FR, ANSWER)                         \
        { CELL *subs_ptr;                                                           \
          /* restore consumer choice point */                                       \
          H = HBREG = PROTECT_FROZEN_H(B);                                          \
          restore_yaam_reg_cpdepth(B);                                              \
          CPREG = B->cp_cp;                                                         \
          ENV = B->cp_env;                                                          \
          /* set_cut(YENV, B->cp_b); --> no effect */                               \
          PREG = (yamop *) CPREG;                                                   \
          PREFETCH_OP(PREG);                                                        \
          /* load answer from table to global stack */                              \
          if (B == DepFr_leader_cp(DEP_FR)) {                                       \
            /*  B is a generator-consumer node  */                                  \
            /* never here if batched scheduling */                                  \
            TABLING_ERROR_CHECKING(generator_consumer, IS_BATCHED_GEN_CP(B));       \
            subs_ptr = (CELL *) (GEN_CP(B) + 1);                                    \
            subs_ptr += SgFr_arity(GEN_CP(B)->cp_sg_fr);                            \
	  } else {                                                                  \
            subs_ptr = (CELL *) (CONS_CP(B) + 1);                                   \
	  }                                                                         \
	  /* printf("consumed answer was %lf \n", ANSWER); */		            \
	  /* subs_ptr = (CELL *) (LOAD_CP(B) + 1);*/		  	            \
	  if (DepFr_last_consumed_term_type(DEP_FR) == MODE_DIRECTED_DIM_INTEGER) { \
	    /* printf (" oooooo2 -> %ld \n", ANSWER); */		\
	    DepFr_last_term_integer(DEP_FR) = ANSWER;		  	            \
            Bind((CELL *) subs_ptr[1], NoTrie_LoadIntegerTerm(ANSWER));	            \
	  } else  /* MODE_DIRECTED_DIM_FLOAT */ {			            \
	    DepFr_last_term_float(DEP_FR) = ANSWER;			            \
            Bind((CELL *) subs_ptr[1], NoTrie_LoadFloatTerm(ANSWER));               \
          }								            \
          /* Bind((CELL *) YENV[1], ANSWER); -- wrong */ /* subs_arity = 1*/        \
          /* --> Bind replaces load_answer(ans_node, YENV PASS_REGS); <--  */       \
          /* procceed */                                                            \
          YENV = ENV;                                                               \
          GONext();                                                                 \
        }
#endif /* THREADS_NO_SUBGOAL_TRIE_MIN_MAX */

#define consume_answer_and_procceed(DEP_FR, ANSWER)                            \
        { CELL *subs_ptr;                                                      \
          /* restore consumer choice point */                                  \
          H = HBREG = PROTECT_FROZEN_H(B);                                     \
          restore_yaam_reg_cpdepth(B);                                         \
          CPREG = B->cp_cp;                                                    \
          ENV = B->cp_env;                                                     \
          /* set_cut(YENV, B->cp_b); --> no effect */                          \
          PREG = (yamop *) CPREG;                                              \
          PREFETCH_OP(PREG);                                                   \
          /* load answer from table to global stack */                         \
          if (B == DepFr_leader_cp(DEP_FR)) {                                  \
            /*  B is a generator-consumer node  */                             \
            /* never here if batched scheduling */                             \
            TABLING_ERROR_CHECKING(generator_consumer, IS_BATCHED_GEN_CP(B));  \
            subs_ptr = (CELL *) (GEN_CP(B) + 1);                               \
            subs_ptr += SgFr_arity(GEN_CP(B)->cp_sg_fr);                       \
	  } else {                                                             \
            subs_ptr = (CELL *) (CONS_CP(B) + 1);                              \
	  }                                                                    \
          FTNA_3_load_answer(ANSWER, subs_ptr);                                \
          /* procceed */                                                       \
          YENV = ENV;                                                          \
          GONext();                                                            \
        }

#define store_loader_node(TAB_ENT, ANSWER)                    \
        { register choiceptr lcp;                             \
	  /* initialize lcp */                                \
          lcp = NORM_CP(LOAD_CP(YENV) - 1);                   \
          /* store loader choice point */                     \
          HBREG = H;                                          \
          store_yaam_reg_cpdepth(lcp);                        \
          lcp->cp_tr = TR;         	                      \
          lcp->cp_ap = LOAD_ANSWER;                           \
          lcp->cp_h  = H;                                     \
          lcp->cp_b  = B;                                     \
          lcp->cp_env= ENV;                                   \
          lcp->cp_cp = CPREG;                                 \
          LOAD_CP(lcp)->cp_last_answer = ANSWER;              \
          store_low_level_trace_info(LOAD_CP(lcp), TAB_ENT);  \
          /* set_cut((CELL *)lcp, B); --> no effect */        \
          B = lcp;                                            \
          YAPOR_SET_LOAD(B);                                  \
          SET_BB(B);                                          \
          TABLING_ERROR_CHECKING_STACK;                       \
        }


#define restore_loader_node(ANSWER)           \
        H = HBREG = PROTECT_FROZEN_H(B);      \
        restore_yaam_reg_cpdepth(B);          \
        CPREG = B->cp_cp;                     \
        ENV = B->cp_env;                      \
        LOAD_CP(B)->cp_last_answer = ANSWER;  \
        SET_BB(PROTECT_FROZEN_B(B))


#define pop_loader_node()             \
        H = PROTECT_FROZEN_H(B);      \
        pop_yaam_reg_cpdepth(B);      \
	CPREG = B->cp_cp;             \
        TABLING_close_alt(B);	      \
        ENV = B->cp_env;              \
	B = B->cp_b;	              \
        HBREG = PROTECT_FROZEN_H(B);  \
        SET_BB(PROTECT_FROZEN_B(B))


#ifdef DEPTH_LIMIT
#define allocate_environment()        \
        YENV[E_CP] = (CELL) CPREG;    \
        YENV[E_E] = (CELL) ENV;       \
        YENV[E_B] = (CELL) B;         \
        YENV[E_DEPTH] = (CELL)DEPTH;  \
        ENV = YENV
#else
#define allocate_environment()        \
        YENV[E_CP] = (CELL) CPREG;    \
        YENV[E_E] = (CELL) ENV;       \
        YENV[E_B] = (CELL) B;         \
        ENV = YENV
#endif /* DEPTH_LIMIT */



/************************************************************************
**                           clause_with_cut                           **
************************************************************************/

#ifdef TABLING_INNER_CUTS
  Op(clause_with_cut, e)
    if (LOCAL_pruning_scope) {
      if (YOUNGER_CP(LOCAL_pruning_scope, B))
        LOCAL_pruning_scope = B;
    } else {
      LOCAL_pruning_scope = B;
      PUT_IN_PRUNING(worker_id);
    }
    PREG = NEXTOP(PREG, e);
    GONext();
  ENDOp();
#endif /* TABLING_INNER_CUTS */


/************************************************************************
**                          table_load_answer                          **
************************************************************************/

  PBOp(table_load_answer, Otapl)
    CELL *subs_ptr;
    ans_node_ptr ans_node;

#ifdef YAPOR
    if (SCH_top_shared_cp(B)) {
#if 0
      PROBLEM: cp_last_answer field is local to the cp!
               -> we need a shared data structure to avoid redundant computations!
      UNLOCK_OR_FRAME(LOCAL_top_or_fr);
#else
      Sfprintf(Serror, "PROBLEM: cp_last_answer field is local to the cp!\n");
      exit(1);
#endif
    }
#endif /* YAPOR */
    subs_ptr = (CELL *) (LOAD_CP(B) + 1);
    ans_node = TrNode_child(LOAD_CP(B)->cp_last_answer);
    if(TrNode_child(ans_node) != NULL) {
      restore_loader_node(ans_node);
    } else {
      pop_loader_node();
    }
    
      
    PREG = (yamop *) CPREG;
    PREFETCH_OP(PREG);
    load_answer(ans_node, subs_ptr PASS_REGS);

    YENV = ENV;
    GONext();
  ENDPBOp();



/************************************************************************
**                          table_try_answer                           **
************************************************************************/

  PBOp(table_try_answer, Otapl)

#ifdef INCOMPLETE_TABLING
    sg_fr_ptr sg_fr;
    ans_node_ptr ans_node;

    sg_fr = GEN_CP(B)->cp_sg_fr;
    ans_node = (ans_node_ptr) TrNode_child(SgFr_try_answer(sg_fr));
    if(ans_node) {
      CELL *subs_ptr = (CELL *) (GEN_CP(B) + 1) + SgFr_arity(sg_fr);

      H = HBREG = PROTECT_FROZEN_H(B);
      restore_yaam_reg_cpdepth(B);
      CPREG = B->cp_cp;
      ENV = B->cp_env;
#ifdef YAPOR
      if (SCH_top_shared_cp(B))
	UNLOCK_OR_FRAME(LOCAL_top_or_fr);
#endif /* YAPOR */
      SET_BB(PROTECT_FROZEN_B(B));

      PREG = (yamop *) CPREG;
      PREFETCH_OP(PREG);
#ifdef THREADS_FULL_SHARING_FTNA_3      
      SgFr_try_answer(sg_fr) = (ans_ref_ptr) ans_node;
      FTNA_3_load_answer((ans_ref_ptr)ans_node, subs_ptr);
#else
      SgFr_try_answer(sg_fr) = ans_node;
      load_answer(ans_node, subs_ptr PASS_REGS);
#endif /* THREADS_FULL_SHARING_FTNA_3 */
      YENV = ENV;
      GONext();
    } else {
      yamop *code_ap;
      PREG = SgFr_code(sg_fr);
      if (PREG->opc == Yap_opcode(_table_try)) {
	/* table_try */
	code_ap = NEXTOP(PREG,Otapl);
	PREG = PREG->u.Otapl.d;
      } else if (PREG->opc == Yap_opcode(_table_try_single)) {
	/* table_try_single */
	code_ap = COMPLETION;
	PREG = PREG->u.Otapl.d;
      } else {
	/* table_try_me */
	code_ap = PREG->u.Otapl.d;
	PREG = NEXTOP(PREG,Otapl);
      }
      PREFETCH_OP(PREG);
      restore_generator_node(SgFr_arity(sg_fr), code_ap);
      YENV = (CELL *) PROTECT_FROZEN_B(B);
      set_cut(YENV, B->cp_b);
      SET_BB(NORM_CP(YENV));
      allocate_environment();
      GONext();
    }
#else
    PREG = PREG->u.Otapl.d;
    PREFETCH_OP(PREG);
    GONext();    
#endif /* INCOMPLETE_TABLING */
  ENDPBOp();




/************************************************************************
**                          table_try_single                           **
************************************************************************/

  PBOp(table_try_single, Otapl)
    tab_ent_ptr tab_ent;
    sg_fr_ptr sg_fr;
    check_trail(TR);
    tab_ent = PREG->u.Otapl.te;
    YENV2MEM;
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD
/*
    struct timespec start, stop;
    if(clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
   }
*/
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);

#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */

    sg_fr = subgoal_search(PREG, YENV_ADDRESS PASS_REGS);

#ifdef EXTRA_STATISTICS_SUBGOAL_SHARING_COMPLETE
    Stats_choice_points[worker_id]++;
    if (SgFr_state(sg_fr) == ready)
      Stats_generator_cp[worker_id]++;
    else if (SgFr_state(sg_fr) == evaluating)
      Stats_consumer_cp[worker_id]++;
    else if (SgFr_state(sg_fr) == complete)  {
      if (SgFr_wid(sg_fr) == worker_id)	
	Stats_completed_cp_private[worker_id]++;
      else
	Stats_completed_cp_shared[worker_id]++;
    }
#endif /* EXTRA_STATISTICS_SUBGOAL_SHARING_COMPLETE */
#ifdef EXTRA_STATISTICS_CHOICE_POINTS
    atomic_inc(&Stats_choice_points);
    if (SgFr_state(sg_fr) == ready) {
      atomic_inc(&Stats_generator_cp);
      LOCK_SG_FR(sg_fr);
      if (SgFr_query_number(sg_fr) == -1)
	SgFr_query_number(sg_fr) = Stats_query_number;
      UNLOCK_SG_FR(sg_fr);
    } else if (SgFr_state(sg_fr) == evaluating)
      atomic_inc(&Stats_consumer_cp);
    else if (SgFr_state(sg_fr) == complete) {
      atomic_inc(&Stats_completed_cp);
      if (SgFr_query_number(sg_fr) != Stats_query_number)
	atomic_inc(&Stats_query_reused_tables);
    }
#endif /* EXTRA_STATISTICS_CHOICE_POINTS */
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD
/*
#define BILLION  1000000000L
    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
     perror( "clock gettime" );
     exit( EXIT_FAILURE );
    }

    walltime_by_thread[walltime_by_thread_run][worker_id] += 
      ( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec ) / BILLION;

#undef BILLION
*/

    gettimeofday(&tv2, NULL);
    walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */

    MEM2YENV;
#ifndef THREADS_SUBGOAL_FRAME_BY_WID
#if defined(THREADS_FULL_SHARING)
    if (SgFr_state(sg_fr) <= ready) {
      LOCK_SG_FR(sg_fr);
      if (SgFr_sg_ent_state(sg_fr) >= complete) {
	SgFr_next_complete(sg_fr) = LOCAL_top_sg_fr_complete;
	LOCAL_top_sg_fr_complete = sg_fr;
      	SgFr_state(sg_fr) = SgFr_sg_ent_state(sg_fr);
      } else 
	SgFr_active_workers(sg_fr)++;      
      UNLOCK_SG_FR(sg_fr);
    } 
#endif /* THREADS_FULL_SHARING */
#endif /*!THREADS_SUBGOAL_FRAME_BY_WID*/

    if (SgFr_state(sg_fr) == ready) {
      /* subgoal new */      
       init_subgoal_frame(sg_fr);
#ifdef DETERMINISTIC_TABLING
      if (IsMode_Batched(TabEnt_mode(tab_ent))) {
	store_deterministic_generator_node(tab_ent, sg_fr);
      } else
#endif /* DETERMINISTIC_TABLING */
      {
	store_generator_node(tab_ent, sg_fr, PREG->u.Otapl.s, COMPLETION);
      }
      PREG = PREG->u.Otapl.d;  /* should work also with PREG = NEXTOP(PREG,Otapl); */
      PREFETCH_OP(PREG);
      allocate_environment();
      GONext();
#ifdef INCOMPLETE_TABLING
    } else if (SgFr_state(sg_fr) == incomplete) {
      /* subgoal incomplete --> start by loading the answers already found */
#ifdef THREADS_FULL_SHARING_FTNA_3 
      ans_ref_ptr ans_ref_node = SgFr_cons_ref_first_ans(sg_fr);
      ans_node_ptr ans_node = TrNode_entry(ans_ref_node);
      SgFr_try_answer(sg_fr) = ans_ref_node;
#else
      ans_node_ptr ans_node = SgFr_first_answer(sg_fr);
      SgFr_try_answer(sg_fr) = ans_node;
#endif  /* THREADS_FULL_SHARING_FTNA_3 */
      CELL *subs_ptr = YENV;
      init_subgoal_frame(sg_fr);
      store_generator_node(tab_ent, sg_fr, PREG->u.Otapl.s, TRY_ANSWER);
      PREG = (yamop *) CPREG;
      PREFETCH_OP(PREG);
      load_answer(ans_node, subs_ptr PASS_REGS);
      YENV = ENV;
      GONext();
#endif /* INCOMPLETE_TABLING */
    } else if (SgFr_state(sg_fr) == evaluating) {
      /* subgoal in evaluation */
      choiceptr leader_cp;
      int leader_dep_on_stack;
      find_dependency_node(sg_fr, leader_cp, leader_dep_on_stack);
      find_leader_node(leader_cp, leader_dep_on_stack);
      store_consumer_node(tab_ent, sg_fr, leader_cp, leader_dep_on_stack);
#ifdef DEBUG_OPTYAP
      if (GLOBAL_parallel_mode == PARALLEL_MODE_RUNNING) {
	choiceptr aux_cp;
	aux_cp = B;
	while (YOUNGER_CP(aux_cp, Get_LOCAL_top_cp_on_stack()))
	  aux_cp = aux_cp->cp_b;
	OPTYAP_ERROR_CHECKING(table_try_single, aux_cp->cp_or_fr != DepFr_top_or_fr(LOCAL_top_dep_fr));
	aux_cp = B;
	while (YOUNGER_CP(aux_cp, DepFr_leader_cp(LOCAL_top_dep_fr)))
	  aux_cp = aux_cp->cp_b;
	OPTYAP_ERROR_CHECKING(table_try_single, aux_cp != DepFr_leader_cp(LOCAL_top_dep_fr));
      }
#endif /* DEBUG_OPTYAP */
      goto answer_resolution;
    } else {
      /* subgoal completed */
#ifdef THREADS_NO_SUBGOAL_TRIE_MIN_MAX
      no_subgoal_trie_pos_ptr no_st_pos = SgFr_no_sg_pos(sg_fr);
      if (no_st_pos != NULL) {
	if (SgNoTrie_answer(no_st_pos) == NULL)
	  /* no answers --> fail */
	  goto fail;
	else /* load answer */ {
          PREG = (yamop *) CPREG;
          PREFETCH_OP(PREG);	  
	  if (SgFr_mode_directed_term_type(sg_fr) == MODE_DIRECTED_DIM_INTEGER)
	    { Bind((CELL *) YENV[1], NoTrie_LoadIntegerTerm((SgNoTrie_answer_integer(no_st_pos))));}
	  else
	    {Bind((CELL *) YENV[1], NoTrie_LoadFloatTerm((SgNoTrie_answer_float(no_st_pos))));}
          //load_answer(ans_node, YENV PASS_REGS);
	  YENV = ENV;
          GONext();
        } 
      }
#endif /* THREADS_NO_SUBGOAL_TRIE_MIN_MAX */
      ans_node_ptr ans_node = SgFr_first_answer(sg_fr);
      if (ans_node == NULL) {
	/* no answers --> fail */
	goto fail;
      } else if (ans_node == SgFr_answer_trie(sg_fr)) {
	/* yes answer --> procceed */
	PREG = (yamop *) CPREG;
	PREFETCH_OP(PREG);
	YENV = ENV;
	GONext();
      } else {
	/* answers -> get first answer */
#ifdef LIMIT_TABLING
	if (SgFr_state(sg_fr) == complete || SgFr_state(sg_fr) == compiled) {
	  SgFr_state(sg_fr)++;  /* complete --> complete_in_use : compiled --> compiled_in_use */
	  remove_from_global_sg_fr_list(sg_fr);
	  TRAIL_FRAME(sg_fr);
	}
#endif /* LIMIT_TABLING */
#if defined(THREADS_FULL_SHARING)
	LOCK_SG_FR(sg_fr);
	if (IsMode_LoadAnswers(TabEnt_mode(tab_ent)) || SgFr_active_workers(sg_fr) > 0) {
#else
        if (IsMode_LoadAnswers(TabEnt_mode(tab_ent))) {
#endif /* THREADS_FULL_SHARING */
          /* load answers from the trie */
	  UNLOCK_SG_FR(sg_fr);
	  if(TrNode_child(ans_node) != NULL) {
	    store_loader_node(tab_ent, ans_node);
	  }
          PREG = (yamop *) CPREG;
          PREFETCH_OP(PREG);
          load_answer(ans_node, YENV PASS_REGS);
	  YENV = ENV;
          GONext();
	} else {
	  /* execute compiled code from the trie */
#if defined(THREADS_FULL_SHARING)
	  if (SgFr_sg_ent_state(sg_fr) < compiled)
#else
          if (SgFr_state(sg_fr) < compiled)
#endif /* THREADS_FULL_SHARING */
	    update_answer_trie(sg_fr PASS_REGS);
	  UNLOCK_SG_FR(sg_fr);
	  PREG = (yamop *) TrNode_child(SgFr_answer_trie(sg_fr));
	  PREFETCH_OP(PREG);
	  *--YENV = 0;  /* vars_arity */
	  *--YENV = 0;  /* heap_arity */
	  GONext();
	}
      }
    }
  ENDPBOp();



/************************************************************************
**                            table_try_me                             **
************************************************************************/

  PBOp(table_try_me, Otapl)
    tab_ent_ptr tab_ent;
    sg_fr_ptr sg_fr;
    check_trail(TR);
    tab_ent = PREG->u.Otapl.te;
    YENV2MEM;
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
    sg_fr = subgoal_search(PREG, YENV_ADDRESS PASS_REGS);

#ifdef EXTRA_STATISTICS_SUBGOAL_SHARING_COMPLETE
    Stats_choice_points[worker_id]++;
    if (SgFr_state(sg_fr) == ready)
      Stats_generator_cp[worker_id]++;
    else if (SgFr_state(sg_fr) == evaluating)
      Stats_consumer_cp[worker_id]++;
    else if (SgFr_state(sg_fr) == complete)  {
      if (SgFr_wid(sg_fr) == worker_id)	
	Stats_completed_cp_private[worker_id]++;
      else
	Stats_completed_cp_shared[worker_id]++;
    }      
#endif /* EXTRA_STATISTICS_SUBGOAL_SHARING_COMPLETE */

#ifdef EXTRA_STATISTICS_CHOICE_POINTS
    atomic_inc(&Stats_choice_points);
    if (SgFr_state(sg_fr) == ready) {
      atomic_inc(&Stats_generator_cp);
      LOCK_SG_FR(sg_fr);
      if (SgFr_query_number(sg_fr) == -1)
	SgFr_query_number(sg_fr) = Stats_query_number;
      UNLOCK_SG_FR(sg_fr);
    } else if (SgFr_state(sg_fr) == evaluating)
      atomic_inc(&Stats_consumer_cp);
    else if (SgFr_state(sg_fr) == complete) {
      atomic_inc(&Stats_completed_cp);
      if (SgFr_query_number(sg_fr) != Stats_query_number)
	atomic_inc(&Stats_query_reused_tables);
    }
#endif /* EXTRA_STATISTICS_CHOICE_POINTS */
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD
    gettimeofday(&tv2, NULL);
    walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */

    MEM2YENV;
#ifndef THREADS_SUBGOAL_FRAME_BY_WID
#if defined(THREADS_FULL_SHARING)
    if (SgFr_state(sg_fr) <= ready) {
      LOCK_SG_FR(sg_fr);
      if (SgFr_sg_ent_state(sg_fr) >= complete) {
	SgFr_next_complete(sg_fr) = LOCAL_top_sg_fr_complete;
	LOCAL_top_sg_fr_complete = sg_fr;
	SgFr_state(sg_fr) = SgFr_sg_ent_state(sg_fr);
      }else 
	SgFr_active_workers(sg_fr)++;
      UNLOCK_SG_FR(sg_fr);
    }
#endif /* THREADS_FULL_SHARING */
#endif /*!THREADS_SUBGOAL_FRAME_BY_WID */
    if (SgFr_state(sg_fr) == ready) {
      /* subgoal new */
      init_subgoal_frame(sg_fr);
      store_generator_node(tab_ent, sg_fr, PREG->u.Otapl.s, PREG->u.Otapl.d);
      PREG = NEXTOP(PREG, Otapl);
      PREFETCH_OP(PREG);
      allocate_environment();
      GONext();
#ifdef INCOMPLETE_TABLING
    } else if (SgFr_state(sg_fr) == incomplete) {
      /* subgoal incomplete --> start by loading the answers already found */
#ifdef THREADS_FULL_SHARING_FTNA_3 
      ans_ref_ptr ans_ref_node = SgFr_cons_ref_first_ans(sg_fr);
      ans_node_ptr ans_node = TrNode_entry(ans_ref_node);
      SgFr_try_answer(sg_fr) = ans_ref_node;
#else
      ans_node_ptr ans_node = SgFr_first_answer(sg_fr);
      SgFr_try_answer(sg_fr) = ans_node;
#endif  /* THREADS_FULL_SHARING_FTNA_3 */
      CELL *subs_ptr = YENV;
      init_subgoal_frame(sg_fr);
      store_generator_node(tab_ent, sg_fr, PREG->u.Otapl.s, TRY_ANSWER);
      PREG = (yamop *) CPREG;
      PREFETCH_OP(PREG);
      load_answer(ans_node, subs_ptr PASS_REGS);
      YENV = ENV;
      GONext();
#endif /* INCOMPLETE_TABLING */
    } else if (SgFr_state(sg_fr) == evaluating) {
      /* subgoal in evaluation */
      choiceptr leader_cp;
      int leader_dep_on_stack;
      find_dependency_node(sg_fr, leader_cp, leader_dep_on_stack);
      find_leader_node(leader_cp, leader_dep_on_stack);
      store_consumer_node(tab_ent, sg_fr, leader_cp, leader_dep_on_stack);
#ifdef DEBUG_OPTYAP
      if (GLOBAL_parallel_mode == PARALLEL_MODE_RUNNING) {
	choiceptr aux_cp;
	aux_cp = B;
	while (YOUNGER_CP(aux_cp, Get_LOCAL_top_cp_on_stack()))
	  aux_cp = aux_cp->cp_b;
	OPTYAP_ERROR_CHECKING(table_try_me, aux_cp->cp_or_fr != DepFr_top_or_fr(LOCAL_top_dep_fr));
	aux_cp = B;
	while (YOUNGER_CP(aux_cp, DepFr_leader_cp(LOCAL_top_dep_fr)))
	  aux_cp = aux_cp->cp_b;
	OPTYAP_ERROR_CHECKING(table_try_me, aux_cp != DepFr_leader_cp(LOCAL_top_dep_fr));
      }
#endif /* DEBUG_OPTYAP */
      goto answer_resolution;
    } else {
      /* subgoal completed */
#ifdef THREADS_NO_SUBGOAL_TRIE_MIN_MAX
      no_subgoal_trie_pos_ptr no_st_pos = SgFr_no_sg_pos(sg_fr);
      if (no_st_pos != NULL) {
	if (SgNoTrie_answer(no_st_pos) == NULL)
	  /* no answers --> fail */
	  goto fail;
	else /* load answer */ {
          PREG = (yamop *) CPREG;
          PREFETCH_OP(PREG);	  
	  if (SgFr_mode_directed_term_type(sg_fr) == MODE_DIRECTED_DIM_INTEGER)
	    {Bind((CELL *) YENV[1], NoTrie_LoadIntegerTerm(SgNoTrie_answer_integer(no_st_pos)));}
	  else
	    {Bind((CELL *) YENV[1], NoTrie_LoadFloatTerm(SgNoTrie_answer_float(no_st_pos)));}
          //load_answer(ans_node, YENV PASS_REGS);
	  YENV = ENV;
          GONext();
        } 
      }
#endif /* THREADS_NO_SUBGOAL_TRIE_MIN_MAX */
      ans_node_ptr ans_node = SgFr_first_answer(sg_fr);

      if (ans_node == NULL) {
	/* no answers --> fail */
	goto fail;
      } else if (ans_node == SgFr_answer_trie(sg_fr)) {
	/* yes answer --> procceed */
	PREG = (yamop *) CPREG;
	PREFETCH_OP(PREG);
	YENV = ENV;
	GONext();
      } else {
	/* answers -> get first answer */
#ifdef LIMIT_TABLING
	if (SgFr_state(sg_fr) == complete || SgFr_state(sg_fr) == compiled) {
	  SgFr_state(sg_fr)++;  /* complete --> complete_in_use : compiled --> compiled_in_use */
	  remove_from_global_sg_fr_list(sg_fr);
	  TRAIL_FRAME(sg_fr);
	}
#endif /* LIMIT_TABLING */
#if defined(THREADS_FULL_SHARING)
	LOCK_SG_FR(sg_fr);
	if (IsMode_LoadAnswers(TabEnt_mode(tab_ent)) || SgFr_active_workers(sg_fr) > 0) {
#else
        if (IsMode_LoadAnswers(TabEnt_mode(tab_ent))) {
#endif /* THREADS_FULL_SHARING */
          /* load answers from the trie */
	  UNLOCK_SG_FR(sg_fr);
	  if(TrNode_child(ans_node) != NULL) {
	    store_loader_node(tab_ent, ans_node);
	  }
          PREG = (yamop *) CPREG;
          PREFETCH_OP(PREG);
          load_answer(ans_node, YENV PASS_REGS);
	  YENV = ENV;
          GONext();
	} else {
	  /* execute compiled code from the trie */
#if defined(THREADS_FULL_SHARING)
	  if (SgFr_sg_ent_state(sg_fr) < compiled)
#else
	  if (SgFr_state(sg_fr) < compiled)
#endif /*THREADS_FULL_SHARING */
	    update_answer_trie(sg_fr PASS_REGS);
	  UNLOCK_SG_FR(sg_fr);
	  PREG = (yamop *) TrNode_child(SgFr_answer_trie(sg_fr));
	  PREFETCH_OP(PREG);
	  *--YENV = 0;  /* vars_arity */
	  *--YENV = 0;  /* heap_arity */
	  GONext();
	}
      }
    }
  ENDPBOp();



/************************************************************************
**                             table_try                               **
************************************************************************/

  PBOp(table_try, Otapl)
    tab_ent_ptr tab_ent;
    sg_fr_ptr sg_fr;

    check_trail(TR);
    tab_ent = PREG->u.Otapl.te;
    YENV2MEM;

#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
    sg_fr = subgoal_search(PREG, YENV_ADDRESS PASS_REGS);

#ifdef EXTRA_STATISTICS_SUBGOAL_SHARING_COMPLETE
    Stats_choice_points[worker_id]++;
    if (SgFr_state(sg_fr) == ready)
      Stats_generator_cp[worker_id]++;
    else if (SgFr_state(sg_fr) == evaluating)
      Stats_consumer_cp[worker_id]++;
    else if (SgFr_state(sg_fr) == complete) {
      if (SgFr_wid(sg_fr) == worker_id)	
	Stats_completed_cp_private[worker_id]++;
      else
	Stats_completed_cp_shared[worker_id]++;
    }

    //    printf(" arg1 = %d  arg2 = %d \n", IntOfTerm(Deref(ARG1)));

#endif /* EXTRA_STATISTICS_SUBGOAL_SHARING_COMPLETE */


#ifdef EXTRA_STATISTICS_CHOICE_POINTS
    atomic_inc(&Stats_choice_points);
    if (SgFr_state(sg_fr) == ready) {
      atomic_inc(&Stats_generator_cp);
      LOCK_SG_FR(sg_fr);
      if (SgFr_query_number(sg_fr) == -1)
	SgFr_query_number(sg_fr) = Stats_query_number;
      UNLOCK_SG_FR(sg_fr);
    } else if (SgFr_state(sg_fr) == evaluating)
      atomic_inc(&Stats_consumer_cp);
    else if (SgFr_state(sg_fr) == complete) {
      atomic_inc(&Stats_completed_cp);
      if (SgFr_query_number(sg_fr) != Stats_query_number)
	atomic_inc(&Stats_query_reused_tables);
    }
#endif /* EXTRA_STATISTICS_CHOICE_POINTS */

#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD
    gettimeofday(&tv2, NULL);
    walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */

    MEM2YENV;
#ifndef THREADS_SUBGOAL_FRAME_BY_WID
#if defined(THREADS_FULL_SHARING)
    if (SgFr_state(sg_fr) <= ready) {
      LOCK_SG_FR(sg_fr);
      if (SgFr_sg_ent_state(sg_fr) >= complete) {
	SgFr_next_complete(sg_fr) = LOCAL_top_sg_fr_complete;
	LOCAL_top_sg_fr_complete = sg_fr;
	SgFr_state(sg_fr) = SgFr_sg_ent_state(sg_fr);
      } else 
	SgFr_active_workers(sg_fr)++;
      UNLOCK_SG_FR(sg_fr);
    }
#endif /*  THREADS_FULL_SHARING */
#endif /* !THREADS_SUBGOAL_FRAME_BY_WID */
    if (SgFr_state(sg_fr) == ready) {
      //      printf("first call in C\n");
      /* subgoal new */
      init_subgoal_frame(sg_fr);
      store_generator_node(tab_ent, sg_fr, PREG->u.Otapl.s, NEXTOP(PREG,Otapl));
      PREG = PREG->u.Otapl.d;
      PREFETCH_OP(PREG);
      allocate_environment();
      GONext();
#ifdef INCOMPLETE_TABLING
    } else if (SgFr_state(sg_fr) == incomplete) {
      /* subgoal incomplete --> start by loading the answers already found */
#ifdef THREADS_FULL_SHARING_FTNA_3 
      ans_ref_ptr ans_ref_node = SgFr_cons_ref_first_ans(sg_fr);
      ans_node_ptr ans_node = TrNode_entry(ans_ref_node);
      SgFr_try_answer(sg_fr) = ans_ref_node;
#else
      ans_node_ptr ans_node = SgFr_first_answer(sg_fr);
      SgFr_try_answer(sg_fr) = ans_node;
#endif  /* THREADS_FULL_SHARING_FTNA_3 */
      CELL *subs_ptr = YENV;
      init_subgoal_frame(sg_fr);
      store_generator_node(tab_ent, sg_fr, PREG->u.Otapl.s, TRY_ANSWER);
      PREG = (yamop *) CPREG;
      PREFETCH_OP(PREG);
      load_answer(ans_node, subs_ptr PASS_REGS);
      YENV = ENV;
      GONext();
#endif /* INCOMPLETE_TABLING */
    } else if (SgFr_state(sg_fr) == evaluating) {
      /* subgoal in evaluation */
      //      printf("second call in C\n");
	//printf("consumer answer is %f \n", FloatOfTerm(SgNoTrie_answer(SgFr_no_sg_pos(sg_fr))));
      choiceptr leader_cp;
      int leader_dep_on_stack;
      find_dependency_node(sg_fr, leader_cp, leader_dep_on_stack);
      find_leader_node(leader_cp, leader_dep_on_stack);
      store_consumer_node(tab_ent, sg_fr, leader_cp, leader_dep_on_stack);
#ifdef DEBUG_OPTYAP
      if (GLOBAL_parallel_mode == PARALLEL_MODE_RUNNING) {
	choiceptr aux_cp;
	aux_cp = B;
	while (YOUNGER_CP(aux_cp, Get_LOCAL_top_cp_on_stack()))
	  aux_cp = aux_cp->cp_b;
	OPTYAP_ERROR_CHECKING(table_try, aux_cp->cp_or_fr != DepFr_top_or_fr(LOCAL_top_dep_fr));
	aux_cp = B;
	while (YOUNGER_CP(aux_cp, DepFr_leader_cp(LOCAL_top_dep_fr)))
	  aux_cp = aux_cp->cp_b;
	OPTYAP_ERROR_CHECKING(table_try, aux_cp != DepFr_leader_cp(LOCAL_top_dep_fr));
      }
#endif /* DEBUG_OPTYAP */
      goto answer_resolution;
    } else {
      /* subgoal completed */
#ifdef THREADS_NO_SUBGOAL_TRIE_MIN_MAX
      no_subgoal_trie_pos_ptr no_st_pos = SgFr_no_sg_pos(sg_fr);
      if (no_st_pos != NULL) {
	if (SgNoTrie_answer(no_st_pos) == NULL) {
	  /* no answers --> fail */
	  goto fail;
	} else /* load answer */ {
          PREG = (yamop *) CPREG;
          PREFETCH_OP(PREG);	  
	  if (SgFr_mode_directed_term_type(sg_fr) == MODE_DIRECTED_DIM_INTEGER)
	    {Bind((CELL *) YENV[1], NoTrie_LoadIntegerTerm(SgNoTrie_answer_integer(no_st_pos)));}
	  else
	    {Bind((CELL *) YENV[1], NoTrie_LoadFloatTerm(SgNoTrie_answer_float(no_st_pos)));}
          //load_answer(ans_node, YENV PASS_REGS);
	  YENV = ENV;
          GONext();
        } 
      }
#endif /* THREADS_NO_SUBGOAL_TRIE_MIN_MAX */

      ans_node_ptr ans_node = SgFr_first_answer(sg_fr);

      //if (worker_id == 0)
	//printf("value1 = %d \n", IntOfTerm(TrNode_entry(SgFr_first_answer(sg_fr))));
	
      //if (IntOfTerm(TrNode_entry(SgFr_first_answer(sg_fr)) == 22501))
	//printf("value2 = %d \n", IntOfTerm(SgNoTrie_answer(SgFr_no_sg_pos(sg_fr))));
      

      if (ans_node == NULL) {
	/* no answers --> fail */
	goto fail;
      } else if (ans_node == SgFr_answer_trie(sg_fr)) {
	/* yes answer --> procceed */
	PREG = (yamop *) CPREG;
	PREFETCH_OP(PREG);
	YENV = ENV;
	GONext();
      } else {
        /* answers -> get first answer */
#ifdef LIMIT_TABLING
	if (SgFr_state(sg_fr) == complete || SgFr_state(sg_fr) == compiled) {
	  SgFr_state(sg_fr)++;  /* complete --> complete_in_use : compiled --> compiled_in_use */
	  remove_from_global_sg_fr_list(sg_fr);
	  TRAIL_FRAME(sg_fr);
	}
#endif /* LIMIT_TABLING */
#if defined(THREADS_FULL_SHARING)
	LOCK_SG_FR(sg_fr);
	if (IsMode_LoadAnswers(TabEnt_mode(tab_ent)) || SgFr_active_workers(sg_fr) > 0) {
#else
        if (IsMode_LoadAnswers(TabEnt_mode(tab_ent))) {
#endif /* THREADS_FULL_SHARING */
          /* load answers from the trie */
	  UNLOCK_SG_FR(sg_fr);
	  if(TrNode_child(ans_node) != NULL) {
	    store_loader_node(tab_ent, ans_node);
	  }
          PREG = (yamop *) CPREG;
          PREFETCH_OP(PREG);
          load_answer(ans_node, YENV PASS_REGS);
	  YENV = ENV;
          GONext();
	} else {
	  /* execute compiled code from the trie */
#if defined(THREADS_FULL_SHARING)
	  if (SgFr_sg_ent_state(sg_fr) < compiled)
#else
	  if (SgFr_state(sg_fr) < compiled)
#endif /*THREADS_FULL_SHARING */
	    update_answer_trie(sg_fr PASS_REGS);
	  
	  UNLOCK_SG_FR(sg_fr);
	  PREG = (yamop *) TrNode_child(SgFr_answer_trie(sg_fr));
	  PREFETCH_OP(PREG);
	  *--YENV = 0;  /* vars_arity */
	  *--YENV = 0;  /* heap_arity */
	  GONext();
	}
      }
    }
  ENDPBOp();



/************************************************************************
**                           table_retry_me                            **
************************************************************************/

  Op(table_retry_me, Otapl)
    restore_generator_node(PREG->u.Otapl.s, PREG->u.Otapl.d);
    YENV = (CELL *) PROTECT_FROZEN_B(B);
    set_cut(YENV, B->cp_b);
    SET_BB(NORM_CP(YENV));
    allocate_environment();
    PREG = NEXTOP(PREG,Otapl);
    GONext();
  ENDOp();



/************************************************************************
**                            table_retry                              **
************************************************************************/

  Op(table_retry, Otapl)
    restore_generator_node(PREG->u.Otapl.s, NEXTOP(PREG,Otapl));
    YENV = (CELL *) PROTECT_FROZEN_B(B);
    set_cut(YENV, B->cp_b);
    SET_BB(NORM_CP(YENV));
    allocate_environment();
    PREG = PREG->u.Otapl.d;
    GONext();
  ENDOp();



/************************************************************************
**                           table_trust_me                            **
************************************************************************/

  Op(table_trust_me, Otapl)
    restore_generator_node(PREG->u.Otapl.s, COMPLETION);
#ifdef DETERMINISTIC_TABLING
    if (B_FZ > B && IS_BATCHED_NORM_GEN_CP(B)) {   
      CELL *subs_ptr = (CELL *)(GEN_CP(B) + 1) + PREG->u.Otapl.s;
      choiceptr gcp = NORM_CP(DET_GEN_CP(subs_ptr) - 1);
      sg_fr_ptr sg_fr = GEN_CP(B)->cp_sg_fr; 
      DET_GEN_CP(gcp)->cp_sg_fr = sg_fr;         
      gcp->cp_h     = B->cp_h;
#ifdef DEPTH_LIMIT
      gcp->cp_depth = B->cp_depth;
#endif /* DEPTH_LIMIT */
      gcp->cp_tr    = B->cp_tr;
      gcp->cp_b     = B->cp_b;
      gcp->cp_ap    = B->cp_ap;
      SgFr_gen_cp(sg_fr) = B = gcp;       
    }
#endif /* DETERMINISTIC_TABLING */
    YENV = (CELL *) PROTECT_FROZEN_B(B);
    set_cut(YENV, B->cp_b);
    SET_BB(NORM_CP(YENV));
    allocate_environment();
    PREG = NEXTOP(PREG,Otapl);
    GONext();
  ENDOp();



/************************************************************************
**                            table_trust                              **
************************************************************************/

  Op(table_trust, Otapl)
    restore_generator_node(PREG->u.Otapl.s, COMPLETION);
#ifdef DETERMINISTIC_TABLING
  if (B_FZ > B && IS_BATCHED_NORM_GEN_CP(B)) {    
      CELL *subs_ptr = (CELL *)(GEN_CP(B) + 1) + PREG->u.Otapl.s;
      choiceptr gcp = NORM_CP(DET_GEN_CP(subs_ptr) - 1);
      sg_fr_ptr sg_fr = GEN_CP(B)->cp_sg_fr; 
      DET_GEN_CP(gcp)->cp_sg_fr = sg_fr;         
      gcp->cp_h     = B->cp_h;
#ifdef DEPTH_LIMIT
      gcp->cp_depth = B->cp_depth;
#endif /* DEPTH_LIMIT */
      gcp->cp_tr    = B->cp_tr;
      gcp->cp_b     = B->cp_b;
      gcp->cp_ap    = B->cp_ap;
      SgFr_gen_cp(sg_fr) = B = gcp;
    }
#endif /* DETERMINISTIC_TABLING */
    YENV = (CELL *) PROTECT_FROZEN_B(B);
    set_cut(YENV, B->cp_b);
    SET_BB(NORM_CP(YENV));
    allocate_environment();
    PREG = PREG->u.Otapl.d;
    GONext();
  ENDOp();



/************************************************************************
**                          table_new_answer                           **
************************************************************************/

  PBOp(table_new_answer, s)


    CELL *subs_ptr;
    choiceptr gcp;
    sg_fr_ptr sg_fr;
    ans_node_ptr ans_node;
    
    
    gcp = NORM_CP(YENV[E_B]);
#ifdef DETERMINISTIC_TABLING
    if (IS_DET_GEN_CP(gcp)){  
      sg_fr = DET_GEN_CP(gcp)->cp_sg_fr;
      subs_ptr = (CELL *)(DET_GEN_CP(gcp) + 1) ; 
    } else
#endif /* DETERMINISTIC_TABLING */
    {
      sg_fr = GEN_CP(gcp)->cp_sg_fr;
      subs_ptr = (CELL *)(GEN_CP(gcp) + 1) + PREG->u.s.s;
    }
#if defined(DEBUG_TABLING) && !defined(DETERMINISTIC_TABLING)
    {
      int i, j, arity_args, arity_subs;
      CELL *aux_args;
      CELL *aux_subs;

      arity_args = PREG->u.s.s;
      arity_subs = *subs_ptr;
      aux_args = (CELL *)(GEN_CP(gcp) + 1);
      aux_subs = subs_ptr;
      for (i = 1; i <= arity_subs; i++) {
        Term term_subs = Deref(*(aux_subs + i));
        for (j = 0; j < arity_args; j++) {
          Term term_arg = Deref(*(aux_args + j));
          if (term_subs == term_arg) break;
	}
	TABLING_ERROR_CHECKING(table_new_answer, j == arity_args);
      }
    }
#endif /* DEBUG_TABLING && !DETERMINISTIC_TABLING */
    LOCK_ANSWER_TRIE(sg_fr);

#ifdef MODE_DIRECTED_TABLING
    if (SgFr_mode_directed(sg_fr)) {

#ifdef THREADS_NO_SUBGOAL_TRIE_MIN_MAX
        if (SgFr_no_sg_pos(sg_fr) != NULL) {
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD___
	  struct timeval tv1, tv2;
	  gettimeofday(&tv1, NULL);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
          if (mode_directed_answer_search_no_trie(sg_fr, subs_ptr PASS_REGS) == true 
              && IS_BATCHED_GEN_CP(gcp)) {
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD___
	    gettimeofday(&tv2, NULL);
	    walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
	    /* deallocate and procceed */
	    PREG = (yamop *) YENV[E_CP];
	    PREFETCH_OP(PREG);
	    CPREG = PREG;
	    SREG = YENV;
	    ENV = YENV = (CELL *) YENV[E_E];
	    GONext();
	  } else /* repeated answer or local_scheduling mode */ {
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD___
	    gettimeofday(&tv2, NULL);
	    walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */	    
	    goto fail; 
	  }

        }
#endif /* THREADS_NO_SUBGOAL_TRIE_MIN_MAX */

      ans_node = mode_directed_answer_search(sg_fr, subs_ptr PASS_REGS);
  
      if (ans_node == NULL) {
	/* no answer inserted */
	UNLOCK_ANSWER_TRIE(sg_fr);
	goto fail;
      }
    } else
#endif /* MODE_DIRECTED_TABLING */
     ans_node = answer_search(sg_fr, subs_ptr PASS_REGS);

#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD___________________
    struct timeval tv1, tv2;
    if (worker_id == 1)
      gettimeofday(&tv1, NULL);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */

#ifdef THREADS_FULL_SHARING_FTNA_3
    boolean ans_rep = consumer_trie_check_insert_node(sg_fr, ans_node PASS_REGS);
    if (!IS_ANSWER_LEAF_NODE(ans_node))
      TAG_AS_ANSWER_LEAF_NODE(ans_node);  /* added this -> this might cause overhead */ 

    if (IS_BATCHED_GEN_CP(gcp)) {
      if (ans_rep == true) {
#ifdef TABLING_EARLY_COMPLETION
	if (gcp == PROTECT_FROZEN_B(B) && (*subs_ptr == 0 || gcp->cp_ap == COMPLETION)) {
	  /* if the current generator choice point is the topmost choice point and the current */
	  /* call is deterministic (i.e., the number of substitution variables is zero or      */
	  /* there are no more alternatives) then the current answer is deterministic and we   */
	  /* can perform an early completion and remove the current generator choice point     */
	  private_completion(sg_fr PASS_REGS);
	  B = B->cp_b;
	  SET_BB(PROTECT_FROZEN_B(B));
	} else if (*subs_ptr == 0) {
	  /* if the number of substitution variables is zero, an answer is sufficient to perform */
	  /* an early completion, but the current generator choice point cannot be removed       */
	  mark_as_completed(sg_fr PASS_REGS);
	  if (gcp->cp_ap != NULL)
	    gcp->cp_ap = COMPLETION;
	}
#endif /* TABLING_EARLY_COMPLETION */
        /* deallocate and procceed */
	PREG = (yamop *) YENV[E_CP];
	PREFETCH_OP(PREG);
	CPREG = PREG;
	SREG = YENV;
	ENV = YENV = (CELL *) YENV[E_E];
#ifdef DEPTH_LIMIT
	DEPTH = YENV[E_DEPTH];
#endif /* DEPTH_LIMIT */
	GONext();
      } else 
	goto fail;
    } else {
#ifdef TABLING_EARLY_COMPLETION
      if (*subs_ptr == 0) {
	/* if the number of substitution variables is zero, an answer is sufficient to perform */
	/* an early completion, but the current generator choice point cannot be removed       */
	if (SgFr_state(sg_fr) < complete)
	  mark_as_completed(sg_fr  PASS_REGS);
	if (gcp->cp_ap != ANSWER_RESOLUTION)
	  gcp->cp_ap = COMPLETION;
      }
#endif /* TABLING_EARLY_COMPLETION */
      goto fail;
    }
      
#endif /* THREADS_FULL_SHARING_FTNA_3 */
    
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD____________
    gettimeofday(&tv2, NULL);
    walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */


    if (! IS_ANSWER_LEAF_NODE(ans_node)) { 
      /* new answer */
#ifdef EXTRA_STATISTICS
      Stats_new_answers++;
#endif
#ifdef TABLING_INNER_CUTS
      /* check for potencial prunings */
      if (! BITMAP_empty(GLOBAL_bm_pruning_workers)) {
        int until_depth, depth;

        until_depth = OrFr_depth(SgFr_gen_top_or_fr(sg_fr));
        depth = OrFr_depth(LOCAL_top_or_fr);
        if (depth > until_depth) {
          int i, ltt;
          bitmap prune_members, members;
          or_fr_ptr leftmost_or_fr, or_fr, nearest_or_fr;

          BITMAP_copy(prune_members, GLOBAL_bm_pruning_workers);
          BITMAP_delete(prune_members, worker_id);
          ltt = BRANCH_LTT(worker_id, depth);
          BITMAP_intersection(members, prune_members, OrFr_members(LOCAL_top_or_fr));
          if (members) {
            for (i = 0; i < GLOBAL_number_workers; i++) {
              if (BITMAP_member(members, i) && 
                  BRANCH_LTT(i, depth) > ltt && 
                  EQUAL_OR_YOUNGER_CP(Get_LOCAL_top_cp(), REMOTE_pruning_scope(i))) {
                leftmost_or_fr = LOCAL_top_or_fr;
  pending_table_new_answer:
		UNLOCK_ANSWER_NODE(ans_node);
                UNLOCK_ANSWER_TRIE(sg_fr);
                LOCK_OR_FRAME(leftmost_or_fr);
                if (Get_LOCAL_prune_request()) {
                  UNLOCK_OR_FRAME(leftmost_or_fr);
                  SCHEDULER_GET_WORK();
                } else {
                  CUT_store_tg_answer(leftmost_or_fr, ans_node, gcp, ltt);
                  UNLOCK_OR_FRAME(leftmost_or_fr);
                }
		if (IS_BATCHED_GEN_CP(gcp)) {
                  /* deallocate and procceed */
                  PREG = (yamop *) YENV[E_CP];
                  PREFETCH_OP(PREG);
                  CPREG = PREG;
                  SREG = YENV;
                  ENV = YENV = (CELL *) YENV[E_E];
#ifdef DEPTH_LIMIT
		  DEPTH = YENV[E_DEPTH];
#endif /* DEPTH_LIMIT */
                  GONext();
		} else {
                  /* fail */
                  goto fail;
		}
              }
	    }
            BITMAP_minus(prune_members, members);
	  }
          leftmost_or_fr = OrFr_nearest_leftnode(LOCAL_top_or_fr);
          depth = OrFr_depth(leftmost_or_fr);
          if (depth > until_depth) {
            ltt = BRANCH_LTT(worker_id, depth);
            BITMAP_intersection(members, prune_members, OrFr_members(leftmost_or_fr));
            if (members) {
              for (i = 0; i < GLOBAL_number_workers; i++) {
                if (BITMAP_member(members, i) &&
                    BRANCH_LTT(i, depth) > ltt &&
                    EQUAL_OR_YOUNGER_CP(GetOrFr_node(leftmost_or_fr), REMOTE_pruning_scope(i)))
                  goto pending_table_new_answer;
	      }
              BITMAP_minus(prune_members, members);
            }
            /* reaching that point we should update the nearest leftnode data */
            leftmost_or_fr = OrFr_nearest_leftnode(leftmost_or_fr);
            depth = OrFr_depth(leftmost_or_fr);
            while (depth > until_depth) {
              ltt = BRANCH_LTT(worker_id, depth);
              BITMAP_intersection(members, prune_members, OrFr_members(leftmost_or_fr));
              if (members) {
                for (i = 0; i < GLOBAL_number_workers; i++) {
                  if (BITMAP_member(members, i) &&
                      BRANCH_LTT(i, depth) > ltt &&
                      EQUAL_OR_YOUNGER_CP(GetOrFr_node(leftmost_or_fr), REMOTE_pruning_scope(i))) {
                    /* update nearest leftnode data */
                    or_fr = LOCAL_top_or_fr;
                    nearest_or_fr = OrFr_nearest_leftnode(or_fr);
                    while (OrFr_depth(nearest_or_fr) > depth) {
                      LOCK_OR_FRAME(or_fr);
                      OrFr_nearest_leftnode(or_fr) = leftmost_or_fr;
                      UNLOCK_OR_FRAME(or_fr);
                      or_fr = nearest_or_fr;
                      nearest_or_fr = OrFr_nearest_leftnode(or_fr);
                    }
                    goto pending_table_new_answer;
  	       	  }
		}
		BITMAP_minus(prune_members, members);
              }
              leftmost_or_fr = OrFr_nearest_leftnode(leftmost_or_fr);
              depth = OrFr_depth(leftmost_or_fr);
            }
            /* update nearest leftnode data */
            or_fr = LOCAL_top_or_fr;
            nearest_or_fr = OrFr_nearest_leftnode(or_fr);
            while (OrFr_depth(nearest_or_fr) > depth) {
              LOCK_OR_FRAME(or_fr);
              OrFr_nearest_leftnode(or_fr) = leftmost_or_fr;
              UNLOCK_OR_FRAME(or_fr);
              or_fr = nearest_or_fr;
              nearest_or_fr = OrFr_nearest_leftnode(or_fr);
            }
          }
        }
      }

      /* check for prune requests */
      if (Get_LOCAL_prune_request()) {
	UNLOCK_ANSWER_NODE(ans_node);
	UNLOCK_ANSWER_TRIE(sg_fr);
        SCHEDULER_GET_WORK();
      }
#endif /* TABLING_INNER_CUTS */
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD_____________
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */

#ifdef THREADS_FULL_SHARING_FTNA
    if (SgFr_first_answer(sg_fr) == NULL) {
      if (BOOL_CAS(&(SgFr_first_answer(sg_fr)), NULL, ans_node)) {
	TAG_AS_ANSWER_LEAF_NODE(ans_node);
	SgFr_last_answer(sg_fr) = ans_node;
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD_____________________
	if (worker_id == 1) {
	  gettimeofday(&tv2, NULL);
	  walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
	}
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
	goto fail;
      }
    }
	ans_node_ptr last_answer;
	
    if (SgFr_last_answer(sg_fr) == NULL) {
      SgFr_last_answer(sg_fr) = last_answer = SgFr_first_answer(sg_fr);
      if (last_answer == ans_node) {
	TAG_AS_ANSWER_LEAF_NODE(ans_node);
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD________________
	if (worker_id == 1) {
	  gettimeofday(&tv2, NULL);
	  walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
	}
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
	goto fail;
      }	
    } else
      last_answer = SgFr_last_answer(sg_fr);
    
    do {
      if (TrNode_child(last_answer)) {
	do {
	  last_answer = TrNode_child(last_answer);
	  if (last_answer == ans_node) {
	    TAG_AS_ANSWER_LEAF_NODE(ans_node);
	    SgFr_last_answer(sg_fr) = last_answer;
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD__________________
	if (worker_id == 1) {
	  gettimeofday(&tv2, NULL);
	  walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
	}
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
	    goto fail;
	  }
	} while(TrNode_child(last_answer));
      }
      
      if (BOOL_CAS(&(TrNode_child(last_answer)), NULL, ans_node)) {
	TAG_AS_ANSWER_LEAF_NODE(ans_node);
	SgFr_last_answer(sg_fr) = ans_node;
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD__________________
	if (worker_id == 1) {
	gettimeofday(&tv2, NULL);
	walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
	}
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
	goto fail;      
      }
    } while(1);
#else  /* !THREADS_FULL_SHARING_FTNA */

#ifndef ANSWER_TRIE_LOCK_AT_ENTRY_LEVEL
      LOCK_SG_FR(sg_fr);     
#endif /* ! ANSWER_TRIE_LOCK_AT_ENTRY_LEVEL */
      if (!IS_ANSWER_LEAF_NODE(ans_node)) {
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02
	ans_node_ptr  ans_node_aux, sg_ans_trie;
	ans_node_aux = ans_node;
	sg_ans_trie = SgFr_answer_trie(sg_fr);
	int invalid_ans = FALSE; 
	while(ans_node_aux != sg_ans_trie && IS_ANSWER_TEMP_NODE(ans_node_aux)) {
	  UNTAG_ANSWER_TEMP_NODE(ans_node_aux);
	  if (IS_INTRA_ANSWER_INVALID_NODE(ans_node_aux))	    
	    invalid_ans = TRUE; 
	  ans_node_aux = TrNode_parent(ans_node_aux);
	}
	
	if (invalid_ans == TRUE) {
	  /* do not add the answer to the chain */
	  UNLOCK_SG_FR(sg_fr);
	  goto fail;
	}
#endif  /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */


#ifdef THREADS_FULL_SHARING
	if (IsMode_Batched(TabEnt_mode(SgFr_tab_ent(sg_fr)))) {	
	  ANSWER_LEAF_NODE_INSTR_RELATIVE(ans_node);
	  if (worker_id < ANSWER_LEAF_NODE_MAX_THREADS)
	    ANSWER_LEAF_NODE_SET_WID(ans_node,worker_id);
	}
#endif /* THREADS_FULL_SHARING */
	if (SgFr_last_answer(sg_fr) == NULL)
	  SgFr_first_answer(sg_fr) = ans_node;	    
	else 
	  TrNode_child(SgFr_last_answer(sg_fr)) = ans_node;
	SgFr_last_answer(sg_fr) = ans_node;
	TAG_AS_ANSWER_LEAF_NODE(ans_node);
      }
#ifdef DEBUG_TABLING
      { 
        ans_node_ptr aux_ans_node = SgFr_first_answer(sg_fr);
        while (aux_ans_node != SgFr_last_answer(sg_fr)) {
	  TABLING_ERROR_CHECKING(table_new_answer, !IS_ANSWER_LEAF_NODE(aux_ans_node));
          aux_ans_node = TrNode_child(aux_ans_node);
        }
      }
#endif /* DEBUG_TABLING */
      UNLOCK_SG_FR(sg_fr);

#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD______________________
	if (worker_id == 1) {
	  gettimeofday(&tv2, NULL);
	  walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
	}
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
#endif /*THREADS_FULL_SHARING_FTNA */


      if (IS_BATCHED_GEN_CP(gcp)) {
#ifdef TABLING_EARLY_COMPLETION
	if (gcp == PROTECT_FROZEN_B(B) && (*subs_ptr == 0 || gcp->cp_ap == COMPLETION)) {
	  /* if the current generator choice point is the topmost choice point and the current */
	  /* call is deterministic (i.e., the number of substitution variables is zero or      */
	  /* there are no more alternatives) then the current answer is deterministic and we   */
	  /* can perform an early completion and remove the current generator choice point     */
	  private_completion(sg_fr PASS_REGS);
	  B = B->cp_b;
	  SET_BB(PROTECT_FROZEN_B(B));
	} else if (*subs_ptr == 0) {
	  /* if the number of substitution variables is zero, an answer is sufficient to perform */
          /* an early completion, but the current generator choice point cannot be removed       */
	  mark_as_completed(sg_fr  PASS_REGS);
	  if (gcp->cp_ap != NULL)
	    gcp->cp_ap = COMPLETION;
	}
#endif /* TABLING_EARLY_COMPLETION */
        /* deallocate and procceed */
        PREG = (yamop *) YENV[E_CP];
        PREFETCH_OP(PREG);
        CPREG = PREG;
        SREG = YENV;
        ENV = YENV = (CELL *) YENV[E_E];
#ifdef DEPTH_LIMIT
	DEPTH = YENV[E_DEPTH];
#endif /* DEPTH_LIMIT */
        GONext();
      } else {
#ifdef TABLING_EARLY_COMPLETION
	if (*subs_ptr == 0) {
	  /* if the number of substitution variables is zero, an answer is sufficient to perform */
          /* an early completion, but the current generator choice point cannot be removed       */
	  mark_as_completed(sg_fr  PASS_REGS);
	  if (gcp->cp_ap != ANSWER_RESOLUTION)
	    gcp->cp_ap = COMPLETION;
	}
#endif /* TABLING_EARLY_COMPLETION */
        /* fail */
        goto fail;
      }
    } else {
      /* repeated answer */
#ifdef EXTRA_STATISTICS
      Stats_repeated_answers++;
#endif
      UNLOCK_ANSWER_TRIE(sg_fr);
#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD____________________
      	if (worker_id == 1) {
      gettimeofday(&tv2, NULL);
      walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
    }
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
      goto fail;
    }
  ENDPBOp();




/************************************************************************
**                      table_answer_resolution                        **
************************************************************************/

  BOp(table_answer_resolution, Otapl)
#ifdef YAPOR
    if (SCH_top_shared_cp(B)) {
      UNLOCK_OR_FRAME(LOCAL_top_or_fr);
    }
#endif /* YAPOR */

  answer_resolution:
    INIT_PREFETCH()
    dep_fr_ptr dep_fr;
    dep_fr = CONS_CP(B)->cp_dep_fr;

#ifdef THREADS_NO_SUBGOAL_TRIE_MIN_MAX


    if (DepFr_no_sg_pos(dep_fr) != NULL) {
      if (SgNoTrie_answer(DepFr_no_sg_pos(dep_fr)) != NULL) {
	/* if the subgoal call has at least one answer */
	if (DepFr_last_consumed_term_type(dep_fr) == MODE_DIRECTED_DIM_INTEGER) {
	  //printf("2-dep_fr = %p %p\n", dep_fr, DepFr_no_sg_pos(dep_fr));

	  if (DepFr_last_term_integer(dep_fr) != SgNoTrie_answer_integer(DepFr_no_sg_pos(dep_fr))) {
	    //printf("passed here -1\n");
	    /* unconsumed answer in dependency frame */
	    consume_answer_and_procceed_no_trie(dep_fr, 
						SgNoTrie_answer_integer(DepFr_no_sg_pos(dep_fr)));
	  }
	} else /* DepFr_last_consumed_term_type(DEP_FR) == MODE_DIRECTED_DIM_FLOAT */ {
	  if (DepFr_last_term_float(dep_fr) != SgNoTrie_answer_float(DepFr_no_sg_pos(dep_fr))) {
	    /* unconsumed answer in dependency frame */
	    consume_answer_and_procceed_no_trie(dep_fr, 
						SgNoTrie_answer_float(DepFr_no_sg_pos(dep_fr)));
	  }
	}
      }
      //printf("passed here -2\n");
      /* no unconsumed answers */
      if (DepFr_backchain_cp(dep_fr) == NULL) {
	/* normal backtrack */
	B = B->cp_b;
	goto fail;
      } else {
	/* chain backtrack */
	choiceptr top_chain_cp, chain_cp;
	/* find chain choice point to backtrack */
	top_chain_cp = DepFr_backchain_cp(dep_fr);
	chain_cp = DepFr_leader_cp(LOCAL_top_dep_fr);
	if (YOUNGER_CP(top_chain_cp, chain_cp))
	  chain_cp = top_chain_cp;
	/* check for dependency frames with unconsumed answers */
	dep_fr = DepFr_next(dep_fr);
	while (YOUNGER_CP(DepFr_cons_cp(dep_fr), chain_cp)) {
	  if (SgNoTrie_answer(DepFr_no_sg_pos(dep_fr)) != NULL) {
	    /* if the subgoal call has at least one answer */
	    
	    if (DepFr_last_consumed_term_type(dep_fr) == MODE_DIRECTED_DIM_INTEGER) {
	      if (DepFr_last_term_integer(dep_fr) != SgNoTrie_answer_integer(DepFr_no_sg_pos(dep_fr))) {
		/* unconsumed answer in dependency frame */
		/* restore bindings, update registers, consume answer and procceed */
		restore_bindings(B->cp_tr, chain_cp->cp_tr);
		B = chain_cp;
		TR = TR_FZ;
		TRAIL_LINK(B->cp_tr);      
		
		consume_answer_and_procceed_no_trie(dep_fr, 
						    SgNoTrie_answer_integer(DepFr_no_sg_pos(dep_fr)));
	      }
	    } else /* DepFr_last_consumed_term_type(DEP_FR) == MODE_DIRECTED_DIM_FLOAT */ {
	      if (DepFr_last_term_float(dep_fr) != SgNoTrie_answer_float(DepFr_no_sg_pos(dep_fr))) {
		/* unconsumed answer in dependency frame */
		/* restore bindings, update registers, consume answer and procceed */
		restore_bindings(B->cp_tr, chain_cp->cp_tr);
		B = chain_cp;
		TR = TR_FZ;
		TRAIL_LINK(B->cp_tr);
		
		consume_answer_and_procceed_no_trie(dep_fr, 
						    SgNoTrie_answer_float(DepFr_no_sg_pos(dep_fr)));
	      }
	    }
	  }
	    dep_fr = DepFr_next(dep_fr);
        }
	/* no dependency frames with unconsumed answers found */
	/* unbind variables */
	unbind_variables(B->cp_tr, chain_cp->cp_tr);
	if (DepFr_leader_cp(LOCAL_top_dep_fr) == chain_cp && 
           (chain_cp->cp_ap == NULL || chain_cp->cp_ap == ANSWER_RESOLUTION)) {
	  B = chain_cp;
	  TR = TR_FZ;
	  TRAIL_LINK(B->cp_tr);
	  goto completion;
	}
	/* backtrack to chain choice point */
	PREG = chain_cp->cp_ap;
	PREFETCH_OP(PREG);
	B = chain_cp;
	TR = TR_FZ;
	TRAIL_LINK(B->cp_tr);
	GONext();	
      }
    }
#endif /* THREADS_NO_SUBGOAL_TRIE_MIN_MAX */

#ifdef THREADS_FULL_SHARING_FTNA_3
    ans_ref_ptr ans_node;
#else
    ans_node_ptr ans_node;
#endif /* THREADS_FULL_SHARING_FTNA_3 */

    OPTYAP_ERROR_CHECKING(answer_resolution, SCH_top_shared_cp(B) && B->cp_or_fr->alternative != ANSWER_RESOLUTION);
    OPTYAP_ERROR_CHECKING(answer_resolution, !SCH_top_shared_cp(B) && B->cp_ap != ANSWER_RESOLUTION);
    //dep_fr = CONS_CP(B)->cp_dep_fr;
    LOCK_DEP_FR(dep_fr);
    ans_node = DepFr_last_answer(dep_fr);
#ifdef STUDY_TIMESTAMP_MDT
    ans_node_ptr child_node = TrNode_child(ans_node);
    if (child_node && (DepFr_last_term(dep_fr) != TrNode_entry(child_node))) {
      DepFr_last_term(dep_fr) = TrNode_entry(child_node);
    }
#endif

#ifdef TIMESTAMP_MODE_DIRECTED_TABLING
    ans_node_ptr child_node = TrNode_child(ans_node);
    if (child_node && (DepFr_last_term(dep_fr) != TrNode_entry(child_node))) {
      /* unconsumed answer */
      //    printf("answer resolution child_node = %p CONS_CP(DepFr_cons_cp(dep_fr))->entry = %ld TrNode_entry(child_node) = %ld \n", child_node,(long)CONS_CP(DepFr_cons_cp(dep_fr)),(long)TrNode_entry(child_node));
      DepFr_last_term(dep_fr) = TrNode_entry(child_node);
      UNLOCK_DEP_FR(dep_fr);
      ans_node = child_node; 
      consume_answer_and_procceed(dep_fr, ans_node);
    }
    UNLOCK_DEP_FR(dep_fr);    
#else /* !TIMESTAMP_MODE_DIRECTED_TABLING */
    if (TrNode_child(ans_node)) {      
      /* unconsumed answers */
#if defined(THREADS_FULL_SHARING_MODE_DIRECTED_V01) || defined(THREADS_FULL_SHARING_MODE_DIRECTED_V02)
      ans_node_ptr curr_ans_node = TrNode_child(ans_node);
      do {
	ans_node = curr_ans_node;
	curr_ans_node = TrNode_child(curr_ans_node);
      } while (curr_ans_node && IS_ANSWER_INVALID_NODE(curr_ans_node)
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02
		                 && IS_INTRA_ANSWER_INVALID_NODE(curr_ans_node)
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */
	       );

      if (!IS_ANSWER_INVALID_NODE(ans_node) 
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02
	  && !IS_INTRA_ANSWER_INVALID_NODE(ans_node)
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */
	  ) {
	/* valid ans_node */
	DepFr_last_answer(dep_fr) = ans_node;
	UNLOCK_DEP_FR(dep_fr);
	consume_answer_and_procceed(dep_fr, ans_node);
      }
      DepFr_last_answer(dep_fr) = ans_node;
    }
    UNLOCK_DEP_FR(dep_fr);    
#else /* THREADS_FULL_SHARING_MODE_DIRECTED_V01 || THREADS_FULL_SHARING_MODE_DIRECTED_V02 */

#ifdef MODE_DIRECTED_TABLING
      if (IS_ANSWER_INVALID_NODE(TrNode_child(ans_node))) {
	ans_node_ptr old_ans_node;
	old_ans_node = ans_node;
	ans_node = TrNode_child(ans_node);
	do {
	  ans_node = TrNode_child(ans_node);
	} while (IS_ANSWER_INVALID_NODE(ans_node));
	TrNode_child(old_ans_node) = ans_node;
	} else 
#endif /* MODE_DIRECTED_TABLING */
	ans_node = TrNode_child(ans_node);
      DepFr_last_answer(dep_fr) = ans_node;
      UNLOCK_DEP_FR(dep_fr);
      //      printf("ans_resolution consume_ans_node = %p\n", ans_node);
      consume_answer_and_procceed(dep_fr, ans_node);
    }
    UNLOCK_DEP_FR(dep_fr);
#endif /* !THREADS_FULL_SHARING_MODE_DIRECTED_V01 && !THREADS_FULL_SHARING_MODE_DIRECTED_V02 */

#ifdef YAPOR
    if (B == DepFr_leader_cp(LOCAL_top_dep_fr)) {
      /*  B is a generator-consumer node  **
      ** never here if batched scheduling */
      TABLING_ERROR_CHECKING(answer_resolution, IS_BATCHED_GEN_CP(B));
      goto completion;
    }
#endif /* YAPOR */
#endif /* TIMESTAMP_MODE_DIRECTED_TABLING */

    /* no unconsumed answers */
    if (DepFr_backchain_cp(dep_fr) == NULL) {
      /* normal backtrack */
#ifdef YAPOR
      if (SCH_top_shared_cp(B)) {
        SCHEDULER_GET_WORK();
      }
#endif /* YAPOR */
      B = B->cp_b;
      goto fail;
    } else {
      /* chain backtrack */
      choiceptr top_chain_cp, chain_cp;
#ifdef YAPOR
      or_fr_ptr start_or_fr, end_or_fr;
#endif /* YAPOR */

      /* find chain choice point to backtrack */
      top_chain_cp = DepFr_backchain_cp(dep_fr);
      chain_cp = DepFr_leader_cp(LOCAL_top_dep_fr);
      if (YOUNGER_CP(top_chain_cp, chain_cp))
        chain_cp = top_chain_cp;
      TABLING_ERROR_CHECKING(answer_resolution, EQUAL_OR_YOUNGER_CP(top_chain_cp, B));
      TABLING_ERROR_CHECKING(answer_resolution, EQUAL_OR_YOUNGER_CP(chain_cp, B));

      /* check for dependency frames with unconsumed answers */
      dep_fr = DepFr_next(dep_fr);
      while (YOUNGER_CP(DepFr_cons_cp(dep_fr), chain_cp)) {
        LOCK_DEP_FR(dep_fr);
        ans_node = DepFr_last_answer(dep_fr);
#ifdef STUDY_TIMESTAMP_MDT
	ans_node_ptr child_node = TrNode_child(ans_node);
	if (child_node && (DepFr_last_term(dep_fr) != TrNode_entry(child_node))) {
	  DepFr_last_term(dep_fr) = TrNode_entry(child_node);
	}
#endif

#ifdef TIMESTAMP_MODE_DIRECTED_TABLING
	ans_node_ptr child_node = TrNode_child(ans_node);
	if (child_node && (DepFr_last_term(dep_fr) != TrNode_entry(child_node))) {
	  /* unconsumed answers */
	  DepFr_last_term(dep_fr) = TrNode_entry(child_node);
	  ans_node = child_node; 
#else /* !TIMESTAMP_MODE_DIRECTED_TABLING */
	if (TrNode_child(ans_node)) {
          /* dependency frame with unconsumed answers */
#if defined(THREADS_FULL_SHARING_MODE_DIRECTED_V01) || defined(THREADS_FULL_SHARING_MODE_DIRECTED_V02)
	  ans_node_ptr curr_ans_node = TrNode_child(ans_node);
	  do {
	    ans_node = curr_ans_node;
	    curr_ans_node = TrNode_child(curr_ans_node);
	  } while (curr_ans_node && IS_ANSWER_INVALID_NODE(curr_ans_node) 		   
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02
		                 && IS_INTRA_ANSWER_INVALID_NODE(curr_ans_node)
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */
		   );
	  if (IS_ANSWER_INVALID_NODE(ans_node) 
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02		   
	      || IS_INTRA_ANSWER_INVALID_NODE(ans_node)
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */
	     ) 
	    {
	      DepFr_last_answer(dep_fr) = ans_node;
	      UNLOCK_DEP_FR(dep_fr);
	      dep_fr = DepFr_next(dep_fr);
	      continue;	    
	    }
#else /* !THREADS_FULL_SHARING_MODE_DIRECTED_V01 && !THREADS_FULL_SHARING_MODE_DIRECTED_V02 */

#ifdef MODE_DIRECTED_TABLING
	  if (IS_ANSWER_INVALID_NODE(TrNode_child(ans_node))) {
	    ans_node_ptr old_ans_node;
	    old_ans_node = ans_node;
	    ans_node = TrNode_child(ans_node);
	    do {
  	      ans_node = TrNode_child(ans_node);
	    } while (IS_ANSWER_INVALID_NODE(ans_node));  
	    TrNode_child(old_ans_node) = ans_node;
	  } else
#endif /* MODE_DIRECTED_TABLING */
	    ans_node = TrNode_child(ans_node);
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V01 || THREADS_FULL_SHARING_MODE_DIRECTED_V02 */
          DepFr_last_answer(dep_fr) = ans_node;
#ifdef YAPOR
          if (YOUNGER_CP(DepFr_backchain_cp(dep_fr), top_chain_cp))
#endif /* YAPOR */
#endif /* TIMESTAMP_MODE_DIRECTED_TABLING */
            DepFr_backchain_cp(dep_fr) = top_chain_cp;
          UNLOCK_DEP_FR(dep_fr);
          chain_cp = DepFr_cons_cp(dep_fr);
#ifdef YAPOR
          /* update shared nodes */
          start_or_fr = LOCAL_top_or_fr;
          end_or_fr = DepFr_top_or_fr(dep_fr);
          if (start_or_fr != end_or_fr) {
            LOCAL_top_or_fr = end_or_fr;
            Set_LOCAL_top_cp(GetOrFr_node(end_or_fr));
            do {
              while (YOUNGER_CP(GetOrFr_node(start_or_fr), GetOrFr_node(end_or_fr))) {
                LOCK_OR_FRAME(start_or_fr);
                BITMAP_delete(OrFr_members(start_or_fr), worker_id);
                if (BITMAP_empty(OrFr_members(start_or_fr))) {
                  if (frame_with_suspensions_not_collected(start_or_fr)) {
                    collect_suspension_frames(start_or_fr);
                  }
#ifdef TABLING_INNER_CUTS
                  if (OrFr_tg_solutions(start_or_fr)) {
                    tg_sol_fr_ptr tg_solutions;
                    or_fr_ptr leftmost_until;
                    tg_solutions = OrFr_tg_solutions(start_or_fr);
                    leftmost_until = CUT_leftmost_until(start_or_fr, OrFr_depth(TgSolFr_gen_cp(tg_solutions)->cp_or_fr));
                    OrFr_tg_solutions(start_or_fr) = NULL;
                    UNLOCK_OR_FRAME(start_or_fr);
                    if (leftmost_until) {
                      LOCK_OR_FRAME(leftmost_until);
                      tg_solutions = CUT_store_tg_answers(leftmost_until, tg_solutions,
                                                          BRANCH_LTT(worker_id, OrFr_depth(leftmost_until)));
                      UNLOCK_OR_FRAME(leftmost_until);
                    }
                    CUT_validate_tg_answers(tg_solutions);
                    goto continue_update_loop1;
                  }
#endif /* TABLING_INNER_CUTS */
                }
                UNLOCK_OR_FRAME(start_or_fr);
#ifdef TABLING_INNER_CUTS
  continue_update_loop1:
#endif /* TABLING_INNER_CUTS */
                start_or_fr = OrFr_next(start_or_fr);
  	      }
              while (YOUNGER_CP(GetOrFr_node(end_or_fr), GetOrFr_node(start_or_fr))) {
                LOCK_OR_FRAME(end_or_fr);
                BITMAP_insert(OrFr_members(end_or_fr), worker_id);
                BRANCH(worker_id, OrFr_depth(end_or_fr)) = 1;
                UNLOCK_OR_FRAME(end_or_fr);
                end_or_fr = OrFr_next(end_or_fr);
	      }
    	    } while (start_or_fr != end_or_fr);
            if (Get_LOCAL_prune_request())
              pruning_over_tabling_data_structures(); 	
          }
#endif /* YAPOR */
#ifdef DEBUG_OPTYAP
	  if (GLOBAL_parallel_mode == PARALLEL_MODE_RUNNING) {
	    choiceptr aux_cp;
	    OPTYAP_ERROR_CHECKING(completion, YOUNGER_CP(Get_LOCAL_top_cp(), Get_LOCAL_top_cp_on_stack()));
	    aux_cp = chain_cp;
	    while (aux_cp != Get_LOCAL_top_cp()) {
	      OPTYAP_ERROR_CHECKING(completion, YOUNGER_CP(Get_LOCAL_top_cp(), aux_cp));
	      OPTYAP_ERROR_CHECKING(completion, EQUAL_OR_YOUNGER_CP(Get_LOCAL_top_cp_on_stack(), aux_cp));
	      aux_cp = aux_cp->cp_b;
	    }
	  }
#endif /* DEBUG_OPTYAP */
          /* restore bindings, update registers, consume answer and procceed */
          restore_bindings(B->cp_tr, chain_cp->cp_tr);
	  TABLING_ERROR_CHECKING(answer_resolution, TR != B->cp_tr && !IsPairTerm((CELL)TrailTerm(TR - 1)));
	  TABLING_ERROR_CHECKING(answer_resolution, TR != B->cp_tr && (tr_fr_ptr) RepPair((CELL)TrailTerm(TR - 1)) != B->cp_tr);
          B = chain_cp;
          TR = TR_FZ;
          TRAIL_LINK(B->cp_tr);
          consume_answer_and_procceed(dep_fr, ans_node);
	}
        UNLOCK_DEP_FR(dep_fr);
        dep_fr = DepFr_next(dep_fr);
      }
      /* no dependency frames with unconsumed answers found */
#ifdef YAPOR
      /* update shared nodes */
      if (EQUAL_OR_YOUNGER_CP(Get_LOCAL_top_cp_on_stack(), chain_cp)) {
        end_or_fr = chain_cp->cp_or_fr;
        start_or_fr = LOCAL_top_or_fr;
        if (start_or_fr != end_or_fr) {
          LOCAL_top_or_fr = end_or_fr;
          Set_LOCAL_top_cp(GetOrFr_node(end_or_fr));
          while (start_or_fr != end_or_fr) {
            LOCK_OR_FRAME(start_or_fr);
            BITMAP_delete(OrFr_members(start_or_fr), worker_id);
            if (BITMAP_empty(OrFr_members(start_or_fr))) {
              if (frame_with_suspensions_not_collected(start_or_fr)) {
                collect_suspension_frames(start_or_fr);
              }
#ifdef TABLING_INNER_CUTS
              if (OrFr_tg_solutions(start_or_fr)) {
                tg_sol_fr_ptr tg_solutions;
                or_fr_ptr leftmost_until;
                tg_solutions = OrFr_tg_solutions(start_or_fr);
                leftmost_until = CUT_leftmost_until(start_or_fr, OrFr_depth(TgSolFr_gen_cp(tg_solutions)->cp_or_fr));
                OrFr_tg_solutions(start_or_fr) = NULL;
                UNLOCK_OR_FRAME(start_or_fr);
                if (leftmost_until) {
                  LOCK_OR_FRAME(leftmost_until);
                  tg_solutions = CUT_store_tg_answers(leftmost_until, tg_solutions,
                                                      BRANCH_LTT(worker_id, OrFr_depth(leftmost_until)));
                  UNLOCK_OR_FRAME(leftmost_until);
                }
                CUT_validate_tg_answers(tg_solutions);
                goto continue_update_loop2;
              }
#endif /* TABLING_INNER_CUTS */
            }
            UNLOCK_OR_FRAME(start_or_fr);
#ifdef TABLING_INNER_CUTS
  continue_update_loop2:
#endif /* TABLING_INNER_CUTS */
            start_or_fr = OrFr_next(start_or_fr);
  	  }
          if (Get_LOCAL_prune_request())
            pruning_over_tabling_data_structures(); 
        }
      }
#endif /* YAPOR */
#ifdef DEBUG_OPTYAP
      if (GLOBAL_parallel_mode == PARALLEL_MODE_RUNNING) {
	choiceptr aux_cp;
	OPTYAP_ERROR_CHECKING(completion, YOUNGER_CP(Get_LOCAL_top_cp(), Get_LOCAL_top_cp_on_stack()));
	aux_cp = chain_cp;
	while (aux_cp != Get_LOCAL_top_cp()) {
	  OPTYAP_ERROR_CHECKING(completion, YOUNGER_CP(Get_LOCAL_top_cp(), aux_cp));
	  OPTYAP_ERROR_CHECKING(completion, EQUAL_OR_YOUNGER_CP(Get_LOCAL_top_cp_on_stack(), aux_cp));
	  aux_cp = aux_cp->cp_b;
	}
      }
#endif /* DEBUG_OPTYAP */
      /* unbind variables */
      unbind_variables(B->cp_tr, chain_cp->cp_tr);
      TABLING_ERROR_CHECKING(answer_resolution, TR != B->cp_tr && !IsPairTerm((CELL)TrailTerm(TR - 1)));
      TABLING_ERROR_CHECKING(answer_resolution, TR != B->cp_tr && (tr_fr_ptr) RepPair((CELL)TrailTerm(TR - 1)) != B->cp_tr);
      if (DepFr_leader_cp(LOCAL_top_dep_fr) == chain_cp && (
        /* chain_cp is a leader node AND ... */
#ifdef YAPOR
        /* the leader dependency is not on stack OR ... */
        DepFr_leader_dep_is_on_stack(LOCAL_top_dep_fr) == FALSE ||
        /* the leader dependency is on stack (this means that chain_cp is a generator node) and */
#endif /* YAPOR */
        /*                 there are no unexploited alternatives                 **
        ** (NULL if batched scheduling OR ANSWER_RESOLUTION if local scheduling) */
        chain_cp->cp_ap == NULL || chain_cp->cp_ap == ANSWER_RESOLUTION)) {
        B = chain_cp;
        TR = TR_FZ;
        TRAIL_LINK(B->cp_tr);
        goto completion;
      }
      /* backtrack to chain choice point */
      PREG = chain_cp->cp_ap;
      PREFETCH_OP(PREG);
      B = chain_cp;
      TR = TR_FZ;
      TRAIL_LINK(B->cp_tr);
      GONext();
    }
    END_PREFETCH()
  ENDBOp();


/************************************************************************
**                          table_completion                           **
************************************************************************/

  BOp(table_completion, Otapl)
#ifdef YAPOR
    if (SCH_top_shared_cp(B)) {
      if (IS_BATCHED_GEN_CP(B)) {
        SCH_new_alternative(PREG, NULL);
        if (B != DepFr_leader_cp(LOCAL_top_dep_fr) && EQUAL_OR_YOUNGER_CP(B_FZ, B)) {
          /* not leader on that node */
          SCHEDULER_GET_WORK();
        }
      } else {
        SCH_new_alternative(PREG, ANSWER_RESOLUTION);
        if (B != DepFr_leader_cp(LOCAL_top_dep_fr)) {
          /* not leader on that node */
          SCHEDULER_GET_WORK();
        }
      }
    } else
#endif /* YAPOR */
    {

      if (IS_BATCHED_GEN_CP(B)) {
        B->cp_ap = NULL;
        if (EQUAL_OR_YOUNGER_CP(B_FZ, B) && B != DepFr_leader_cp(LOCAL_top_dep_fr)) {
          /* not leader on that node */
          B = B->cp_b;
          goto fail;
        }
      } else {
        B->cp_ap = ANSWER_RESOLUTION;
        if (B != DepFr_leader_cp(LOCAL_top_dep_fr)) {
          /* not leader on that node */
          B = B->cp_b;
          goto fail;
        }
      }
    }
    /* leader on that node */


  completion:
    INIT_PREFETCH()
    dep_fr_ptr dep_fr;
#ifdef THREADS_FULL_SHARING_FTNA_3
    ans_ref_ptr ans_node;
#else
    ans_node_ptr ans_node;
#endif /* THREADS_FULL_SHARING_FTNA_3 */

#ifdef YAPOR
#ifdef TIMESTAMP_CHECK
    long timestamp = 0;
#endif /* TIMESTAMP_CHECK */
    int entry_owners = 0;

    if (SCH_top_shared_cp(B)) {
#ifdef TIMESTAMP_CHECK
      timestamp = ++GLOBAL_timestamp;
#endif /* TIMESTAMP_CHECK */
      entry_owners = OrFr_owners(LOCAL_top_or_fr);
    }
#endif /* YAPOR */

    /* check for dependency frames with unconsumed answers */
    dep_fr = LOCAL_top_dep_fr;    

#ifdef THREADS_NO_SUBGOAL_TRIE_MIN_MAX
    if (DepFr_no_sg_pos(dep_fr) != NULL) {
      while (YOUNGER_CP(DepFr_cons_cp(dep_fr), B)) {
	if (SgNoTrie_answer(DepFr_no_sg_pos(dep_fr))) {
	  /* if the subgoal call has at least one answer */
	  
	  if (DepFr_last_consumed_term_type(dep_fr) == MODE_DIRECTED_DIM_INTEGER) {
	    if (DepFr_last_term_integer(dep_fr) != SgNoTrie_answer_integer(DepFr_no_sg_pos(dep_fr))) {
	      // unconsumed answer in dependency frame 
	      if (B->cp_ap)
		DepFr_backchain_cp(dep_fr) = B;
	      else
		DepFr_backchain_cp(dep_fr) = B->cp_b;
	      
	      // rebind variables, update registers, consume answer and procceed
	      rebind_variables(DepFr_cons_cp(dep_fr)->cp_tr, B->cp_tr);
	      B = DepFr_cons_cp(dep_fr);
	      TR = TR_FZ;
	      if (TR != B->cp_tr)
		TRAIL_LINK(B->cp_tr);
	      consume_answer_and_procceed_no_trie(dep_fr, SgNoTrie_answer_integer(DepFr_no_sg_pos(dep_fr)));
	    }
	  } else /* DepFr_last_consumed_term_type(DEP_FR) == MODE_DIRECTED_DIM_FLOAT */ {
	    if (DepFr_last_term_float(dep_fr) != SgNoTrie_answer_float(DepFr_no_sg_pos(dep_fr))) {
	      /* unconsumed answer in dependency frame */
	      
	      if (B->cp_ap)
		DepFr_backchain_cp(dep_fr) = B;
	      else
		DepFr_backchain_cp(dep_fr) = B->cp_b;
	      
	      // rebind variables, update registers, consume answer and procceed
	      rebind_variables(DepFr_cons_cp(dep_fr)->cp_tr, B->cp_tr);
	      B = DepFr_cons_cp(dep_fr);
	      TR = TR_FZ;
	      if (TR != B->cp_tr)
		TRAIL_LINK(B->cp_tr);
	      consume_answer_and_procceed_no_trie(dep_fr, SgNoTrie_answer_float(DepFr_no_sg_pos(dep_fr)));
	      
	    }	  
	  }
	}
	dep_fr = DepFr_next(dep_fr);
      }
      /* no dependency frames with unconsumed answers found */
      /* complete all */
      sg_fr_ptr sg_fr;
#ifdef DETERMINISTIC_TABLING
      if (IS_DET_GEN_CP(B))
	sg_fr = DET_GEN_CP(B)->cp_sg_fr;
      else	 
#endif /* DETERMINISTIC_TABLING */
	sg_fr = GEN_CP(B)->cp_sg_fr;
      private_completion(sg_fr PASS_REGS);
      if (IS_BATCHED_GEN_CP(B)) {
        /* backtrack */
        B = B->cp_b;
        SET_BB(PROTECT_FROZEN_B(B));
        goto fail;
      } else {
        /* subgoal completed */
        no_subgoal_trie_pos_ptr no_st_pos = SgFr_no_sg_pos(sg_fr);	
	if (SgNoTrie_answer(no_st_pos) == NULL)
	  /* no answers --> fail */
	  goto fail;
	else /* load answer */ {
	  pop_generator_node(SgFr_arity(sg_fr));
	  PREG = (yamop *) CPREG;
	  PREFETCH_OP(PREG); 

	  //printf("3-FloatOfTerm(term) = %.16lf \n", SgNoTrie_answer_float(no_st_pos));
	  //Term ttt = MkFloatTerm(SgNoTrie_answer_float(no_st_pos));
	  //printf("4-FloatOfTerm(term) = %.15lf \n", FloatOfTerm(ttt));




	  if (SgFr_mode_directed_term_type(sg_fr) == MODE_DIRECTED_DIM_INTEGER)
	    {Bind((CELL *) YENV[1], NoTrie_LoadIntegerTerm(SgNoTrie_answer_integer(no_st_pos)));}
	  else
	    {Bind((CELL *) YENV[1], NoTrie_LoadFloatTerm(SgNoTrie_answer_float(no_st_pos)));}
	  //load_answer(ans_node, YENV PASS_REGS);
	  YENV = ENV;
	  GONext();
	} 
      }
    }   
#endif /* THREADS_NO_SUBGOAL_TRIE_MIN_MAX */

    while (YOUNGER_CP(DepFr_cons_cp(dep_fr), B)) {
      LOCK_DEP_FR(dep_fr);
      ans_node = DepFr_last_answer(dep_fr);

#ifdef STUDY_TIMESTAMP_MDT
    ans_node_ptr child_node = TrNode_child(ans_node);
    if (child_node && (DepFr_last_term(dep_fr) != TrNode_entry(child_node))) {
      DepFr_last_term(dep_fr) = TrNode_entry(child_node);
    }
#endif

#ifdef TIMESTAMP_MODE_DIRECTED_TABLING
	ans_node_ptr child_node = TrNode_child(ans_node);
	if (child_node && (DepFr_last_term(dep_fr) != TrNode_entry(child_node))) {
	  /* dependency frame with unconsumed answers */
	  DepFr_last_term(dep_fr) = TrNode_entry(child_node);
	  ans_node = child_node; 
#else /* !TIMESTAMP_MODE_DIRECTED_TABLING */
      if (TrNode_child(ans_node)) {
        /* dependency frame with unconsumed answers */
#ifdef MODE_DIRECTED_TABLING
	if (IS_ANSWER_INVALID_NODE(TrNode_child(ans_node))) {
	  ans_node_ptr old_ans_node;
	  old_ans_node = ans_node;
	  ans_node = TrNode_child(ans_node);
	  do {
	    ans_node = TrNode_child(ans_node);
	  } while (IS_ANSWER_INVALID_NODE(ans_node));
	  TrNode_child(old_ans_node) = ans_node;
	} else
#endif /* MODE_DIRECTED_TABLING */
	  ans_node = TrNode_child(ans_node);
        DepFr_last_answer(dep_fr) = ans_node;
#endif /* TIMESTAMP_MODE_DIRECTED_TABLING */

        if (B->cp_ap) {
#ifdef YAPOR
          if (YOUNGER_CP(DepFr_backchain_cp(dep_fr), B))
#endif /* YAPOR */
            DepFr_backchain_cp(dep_fr) = B;
	} else {
#ifdef YAPOR
          if (YOUNGER_CP(DepFr_backchain_cp(dep_fr), B->cp_b))
#endif /* YAPOR */
            DepFr_backchain_cp(dep_fr) = B->cp_b;
	}
        UNLOCK_DEP_FR(dep_fr);

#ifdef DEBUG_OPTYAP
        if (GLOBAL_parallel_mode == PARALLEL_MODE_RUNNING) {
	  choiceptr aux_cp;
	  OPTYAP_ERROR_CHECKING(completion, Get_LOCAL_top_cp(), Get_LOCAL_top_cp_on_stack());
	  aux_cp = DepFr_cons_cp(dep_fr);
	  while (YOUNGER_CP(aux_cp, Get_LOCAL_top_cp_on_stack()))
	    aux_cp = aux_cp->cp_b;
	  OPTYAP_ERROR_CHECKING(completion, aux_cp->cp_or_fr != DepFr_top_or_fr(dep_fr));
	}
#endif /* DEBUG_OPTYAP */
#ifdef YAPOR
        /* update shared nodes */
        if (YOUNGER_CP(Get_LOCAL_top_cp_on_stack(), Get_LOCAL_top_cp())) {
          or_fr_ptr or_frame = DepFr_top_or_fr(dep_fr);
          while (or_frame != LOCAL_top_or_fr) {
            LOCK_OR_FRAME(or_frame);
            BITMAP_insert(OrFr_members(or_frame), worker_id);
            BRANCH(worker_id, OrFr_depth(or_frame)) = 1;
            UNLOCK_OR_FRAME(or_frame);
            or_frame = OrFr_next(or_frame);
          }
          LOCAL_top_or_fr = DepFr_top_or_fr(dep_fr);
          Set_LOCAL_top_cp(GetOrFr_node(LOCAL_top_or_fr));
        }
#endif /* YAPOR */
#ifdef DEBUG_OPTYAP
        if (GLOBAL_parallel_mode == PARALLEL_MODE_RUNNING) {
	  choiceptr aux_cp;
	  OPTYAP_ERROR_CHECKING(completion, YOUNGER_CP(Get_LOCAL_top_cp(), Get_LOCAL_top_cp_on_stack()));
	  aux_cp = DepFr_cons_cp(dep_fr);
	  while (aux_cp != Get_LOCAL_top_cp()) {
	    OPTYAP_ERROR_CHECKING(completion, YOUNGER_CP(Get_LOCAL_top_cp(), aux_cp));
	    OPTYAP_ERROR_CHECKING(completion, EQUAL_OR_YOUNGER_CP(Get_LOCAL_top_cp_on_stack(), aux_cp));
	    aux_cp = aux_cp->cp_b;
	  }
	}
#endif /* DEBUG_OPTYAP */
        /* rebind variables, update registers, consume answer and procceed */
	TABLING_ERROR_CHECKING(completion, EQUAL_OR_YOUNGER_CP(B, DepFr_cons_cp(dep_fr)));
	TABLING_ERROR_CHECKING(completion, B->cp_tr > DepFr_cons_cp(dep_fr)->cp_tr);
        rebind_variables(DepFr_cons_cp(dep_fr)->cp_tr, B->cp_tr);
	TABLING_ERROR_CHECKING(completion, TR != B->cp_tr && !IsPairTerm((CELL)TrailTerm(TR - 1)));
	TABLING_ERROR_CHECKING(completion, TR != B->cp_tr && (tr_fr_ptr) RepPair((CELL)TrailTerm(TR - 1)) != B->cp_tr);
        B = DepFr_cons_cp(dep_fr);
        TR = TR_FZ;
        if (TR != B->cp_tr)
          TRAIL_LINK(B->cp_tr);
	//	printf("completion consume_ans_node = %p\n", ans_node);
        consume_answer_and_procceed(dep_fr, ans_node);
      }
      UNLOCK_DEP_FR(dep_fr);
#ifdef TIMESTAMP_CHECK
      DepFr_timestamp(dep_fr) = timestamp;
#endif /* TIMESTAMP_CHECK */
      dep_fr = DepFr_next(dep_fr);
    }

    /* no dependency frames with unconsumed answers found */
#ifdef YAPOR
    if (SCH_top_shared_cp(B)) {
      if (entry_owners > 1) {
        /* more owners when we start looking for dependency frames with unconsumed answers */
        if (YOUNGER_CP(B_FZ, B)) {
          suspend_branch();
          /* check for suspension frames to be resumed */
          while (YOUNGER_CP(GetOrFr_node(LOCAL_top_susp_or_fr), Get_LOCAL_top_cp())) {
            or_fr_ptr susp_or_fr;
            susp_fr_ptr resume_fr;
            susp_or_fr = LOCAL_top_susp_or_fr;
            LOCK_OR_FRAME(susp_or_fr);
#ifdef TIMESTAMP_CHECK
            resume_fr = suspension_frame_to_resume(susp_or_fr, timestamp);
#else
            resume_fr = suspension_frame_to_resume(susp_or_fr);
#endif /* TIMESTAMP_CHECK */
	    if (resume_fr) {
              if (OrFr_suspensions(susp_or_fr) == NULL) {
                LOCAL_top_susp_or_fr = OrFr_nearest_suspnode(susp_or_fr);
                OrFr_nearest_suspnode(susp_or_fr) = susp_or_fr;
              }
              UNLOCK_OR_FRAME(susp_or_fr);
              rebind_variables(GetOrFr_node(susp_or_fr)->cp_tr, B->cp_tr);
              resume_suspension_frame(resume_fr, susp_or_fr);
              B = Get_LOCAL_top_cp();
              SET_BB(B_FZ);
              TR = TR_FZ;
              TRAIL_LINK(B->cp_tr);
              goto completion;
            }
            LOCAL_top_susp_or_fr = OrFr_nearest_suspnode(susp_or_fr);
            OrFr_nearest_suspnode(susp_or_fr) = NULL;
            UNLOCK_OR_FRAME(susp_or_fr);
          }
        }
      } else {
        /* unique owner */
        if (frame_with_suspensions_not_collected(LOCAL_top_or_fr))
          collect_suspension_frames(LOCAL_top_or_fr);
        /* check for suspension frames to be resumed */
        while (EQUAL_OR_YOUNGER_CP(GetOrFr_node(LOCAL_top_susp_or_fr), Get_LOCAL_top_cp())) {
          or_fr_ptr susp_or_fr;
          susp_fr_ptr resume_fr;
          susp_or_fr = LOCAL_top_susp_or_fr;
#ifdef TIMESTAMP_CHECK
          resume_fr = suspension_frame_to_resume(susp_or_fr, timestamp);
#else
          resume_fr = suspension_frame_to_resume(susp_or_fr);
#endif /* TIMESTAMP_CHECK */
          if (resume_fr) {
            if (OrFr_suspensions(susp_or_fr) == NULL) {
              LOCAL_top_susp_or_fr = OrFr_nearest_suspnode(susp_or_fr);
              OrFr_nearest_suspnode(susp_or_fr) = susp_or_fr;
            }
            if (YOUNGER_CP(B_FZ, B)) {
              suspend_branch();
            }
            rebind_variables(GetOrFr_node(susp_or_fr)->cp_tr, B->cp_tr);
            resume_suspension_frame(resume_fr, susp_or_fr);
            B = Get_LOCAL_top_cp();
            SET_BB(B_FZ);
            TR = TR_FZ;
            TRAIL_LINK(B->cp_tr);
            goto completion;
          }
          LOCAL_top_susp_or_fr = OrFr_nearest_suspnode(susp_or_fr);
          OrFr_nearest_suspnode(susp_or_fr) = NULL;
        }
        /* complete all */
        public_completion();
      }
      TABLING_ERROR_CHECKING(completion, TR != B->cp_tr && !IsPairTerm((CELL)TrailTerm(TR - 1)));
      TABLING_ERROR_CHECKING(completion, TR != B->cp_tr && (tr_fr_ptr) RepPair((CELL)TrailTerm(TR - 1)) != B->cp_tr);
      if (B == DepFr_leader_cp(LOCAL_top_dep_fr)) {
        /*  B is a generator-consumer node  */
        /* never here if batched scheduling */
        ans_node_ptr ans_node;
	TABLING_ERROR_CHECKING(completion, IS_BATCHED_GEN_CP(B));
        TR = B->cp_tr;
        SET_BB(B);
        LOCK_OR_FRAME(LOCAL_top_or_fr);
        LOCK_DEP_FR(LOCAL_top_dep_fr);
        ans_node = DepFr_last_answer(LOCAL_top_dep_fr);
        if (TrNode_child(ans_node)) {
          /* unconsumed answers */
          UNLOCK_OR_FRAME(LOCAL_top_or_fr);
#ifdef MODE_DIRECTED_TABLING
	  if (IS_ANSWER_INVALID_NODE(TrNode_child(ans_node))) {
	    ans_node_ptr old_ans_node;
	    old_ans_node = ans_node;
	    ans_node = TrNode_child(ans_node);
	    do {
	      ans_node = TrNode_child(ans_node);
	    } while (IS_ANSWER_INVALID_NODE(ans_node));
	    TrNode_child(old_ans_node) = ans_node;
	  } else
#endif /* MODE_DIRECTED_TABLING */
	    ans_node = TrNode_child(ans_node);
          DepFr_last_answer(LOCAL_top_dep_fr) = ans_node;
          UNLOCK_DEP_FR(LOCAL_top_dep_fr);
          consume_answer_and_procceed(LOCAL_top_dep_fr, ans_node);
        }
        /* no unconsumed answers */
        UNLOCK_DEP_FR(LOCAL_top_dep_fr);
        if (OrFr_owners(LOCAL_top_or_fr) > 1) {
          /* more owners -> move up one node */
          Set_LOCAL_top_cp_on_stack(GetOrFr_node(OrFr_next_on_stack(LOCAL_top_or_fr)));
          BITMAP_delete(OrFr_members(LOCAL_top_or_fr), worker_id);
          OrFr_owners(LOCAL_top_or_fr)--;
          LOCAL_top_dep_fr = DepFr_next(LOCAL_top_dep_fr);
          UNLOCK_OR_FRAME(LOCAL_top_or_fr);
          if (LOCAL_top_sg_fr && Get_LOCAL_top_cp() == SgFr_gen_cp(LOCAL_top_sg_fr)) {
            LOCAL_top_sg_fr = SgFr_next(LOCAL_top_sg_fr);
          }
          SCH_update_local_or_tops();
          CUT_reset_prune_request();
          adjust_freeze_registers();
          goto shared_fail;
        } else {
          /* free top dependency frame --> get work */
          OrFr_alternative(LOCAL_top_or_fr) = NULL;
          UNLOCK_OR_FRAME(LOCAL_top_or_fr);
          dep_fr = DepFr_next(LOCAL_top_dep_fr);
          FREE_DEPENDENCY_FRAME(LOCAL_top_dep_fr);
          LOCAL_top_dep_fr = dep_fr;
          adjust_freeze_registers();
          SCHEDULER_GET_WORK();
        }
      }
      /* goto getwork */
      PREG = B->cp_ap;
      PREFETCH_OP(PREG);
      TR = B->cp_tr;
      SET_BB(B);
      GONext();
    } else
#endif /* YAPOR */
    {
      /* complete all */
      sg_fr_ptr sg_fr;

#ifdef DETERMINISTIC_TABLING
      if (IS_DET_GEN_CP(B))
	sg_fr = DET_GEN_CP(B)->cp_sg_fr;
      else	 
#endif /* DETERMINISTIC_TABLING */
	sg_fr = GEN_CP(B)->cp_sg_fr;

#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD_______________
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
      private_completion(sg_fr PASS_REGS);

#ifdef EXTRA_STATISTICS_WALLTIME_BY_THREAD________________
	gettimeofday(&tv2, NULL);
	walltime_by_thread[walltime_by_thread_run][worker_id] += ((float)(1000000*(tv2.tv_sec - tv1.tv_sec) + tv2.tv_usec - tv1.tv_usec) / 1000);
#endif /* EXTRA_STATISTICS_WALLTIME_BY_THREAD */
      if (IS_BATCHED_GEN_CP(B)) {
        /* backtrack */
        B = B->cp_b;
        SET_BB(PROTECT_FROZEN_B(B));
        goto fail;
      } else {
        /* subgoal completed */
	ans_node_ptr first_node;
	first_node = SgFr_first_answer(sg_fr);

        if (first_node == NULL) {
          /* no answers --> fail */
          B = B->cp_b;
          SET_BB(PROTECT_FROZEN_B(B));
          goto fail;
        }
	TABLING_ERROR_CHECKING(completion, TR != B->cp_tr && !IsPairTerm((CELL)TrailTerm(TR - 1)));
	TABLING_ERROR_CHECKING(completion, TR != B->cp_tr && (tr_fr_ptr) RepPair((CELL)TrailTerm(TR - 1)) != B->cp_tr);
        pop_generator_node(SgFr_arity(sg_fr));
        if (first_node == SgFr_answer_trie(sg_fr)) {
          /* yes answer --> procceed */
          PREG = (yamop *) CPREG;
          PREFETCH_OP(PREG);
          YENV = ENV;
          GONext();
        } else  {
          /* answers -> get first answer */
	  tab_ent_ptr tab_ent = SgFr_tab_ent(sg_fr);
#ifdef LIMIT_TABLING
	  SgFr_state(sg_fr)++;  /* complete --> complete_in_use */
	  remove_from_global_sg_fr_list(sg_fr);
	  TRAIL_FRAME(sg_fr);
#endif /* LIMIT_TABLING */
	  if (IsMode_LoadAnswers(TabEnt_mode(tab_ent))) {
            /* load answers from the trie */
	    if(TrNode_child(first_node) != NULL) {
	      store_loader_node(tab_ent, first_node);
	    }
            PREG = (yamop *) CPREG;
            PREFETCH_OP(PREG);
            load_answer(first_node, YENV PASS_REGS);
	    YENV = ENV;
            GONext();
	  } else {
#if defined(THREADS_FULL_SHARING)
	    LOCK_SG_FR(sg_fr);
	    if (SgFr_active_workers(sg_fr) > 0) {
	      /* load answers from the trie */
	      UNLOCK_SG_FR(sg_fr);
	      if(TrNode_child(first_node) != NULL) {
		store_loader_node(tab_ent, first_node);
	      }
	      PREG = (yamop *) CPREG;
	      PREFETCH_OP(PREG);
	      load_answer(first_node, YENV PASS_REGS);
	      YENV = ENV;
	      GONext();
	    }
#endif /* THREADS_FULL_SHARING */
	    /* execute compiled code from the trie */
#if defined(THREADS_FULL_SHARING)
	    if (SgFr_sg_ent_state(sg_fr) < compiled)
#else
	    LOCK_SG_FR(sg_fr);
	    if (SgFr_state(sg_fr) < compiled)
#endif /* THREADS_FULL_SHARING */
	      update_answer_trie(sg_fr PASS_REGS);	    
	    UNLOCK_SG_FR(sg_fr);
	    PREG = (yamop *) TrNode_child(SgFr_answer_trie(sg_fr));
	    PREFETCH_OP(PREG);
	    *--YENV = 0;  /* vars_arity */
            *--YENV = 0;  /* heap_arity */
	    GONext();
	  }
	}
      }
    }
    END_PREFETCH()
  ENDBOp();
