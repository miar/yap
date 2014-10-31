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

/****************************************************************
**                   Configuration Parameters                  **
****************************************************************/
/* thread safe generation of random numbers */

///#define THREADS_RANDOM_GENERATION  1 

/****************************
**      default sizes      **
****************************/
#define MAX_TABLE_VARS     1000
#define TRIE_LOCK_BUCKETS  512


/************************original - inicio ********************/
///#define THREADS_DIRECT_BUCKETS    32  - good  (27 - bad) (50 -bad) 

#define THREADS_DIRECT_BUCKETS    65
//%%#define THREADS_INDIRECT_BUCKETS  ((MAX_THREADS - THREADS_DIRECT_BUCKETS) / THREADS_DIRECT_BUCKETS)  /* (1024 - 32) / 32 = 31 */ 
#define THREADS_INDIRECT_BUCKETS   0
#define THREADS_NUM_BUCKETS       (THREADS_DIRECT_BUCKETS + THREADS_INDIRECT_BUCKETS)
/************************original - fim ********************/
/*
#define THREADS_DIRECT_BUCKETS    27
#define THREADS_INDIRECT_BUCKETS  0
#define THREADS_NUM_BUCKETS       (THREADS_DIRECT_BUCKETS + THREADS_INDIRECT_BUCKETS)
*/

#define TG_ANSWER_SLOTS    20
#define MAX_BRANCH_DEPTH   1000

/**********************************************************************
**      memory mapping scheme for YapOr (mandatory, define one)      **
**********************************************************************/
#define MMAP_MEMORY_MAPPING_SCHEME 1
/* #define SHM_MEMORY_MAPPING_SCHEME 1 */

/****************************************************************
**      use shared pages memory alloc scheme ? (optional)      **
****************************************************************/

//#define USE_PAGES_MALLOC 1  /* disable with full_sharing + mode_directed_tabling */

/**********************************************************************
**      trail freeze scheme for tabling (mandatory, define one)      **
**********************************************************************/
#define BFZ_TRAIL_SCHEME 1
/* #define BBREG_TRAIL_SCHEME 1 */

/************************************************************************
**      multithreading design for tabling (mandatory, define one)      **
************************************************************************/
#define THREADS_NO_SHARING 1
//#define THREADS_SUBGOAL_SHARING 1
//#define THREADS_FULL_SHARING 1

//#define EXTRA_STATISTICS   1
//#define EXTRA_STATISTICS_CPUTIME_BY_THREAD   1
//#define EXTRA_STATISTICS_WALLTIME_BY_THREAD  1  /* answer_search only */
//#define EXTRA_STATISTICS_CHOICE_POINTS   1

//#define THREADS_FULL_SHARING_FTNA  1    /*  fast table_new_answer (SgFr_last_answer private) - use only with local scheduling. check first for mode_direct_tabling */

///#define THREADS_FULL_SHARING_FTNA_3  1    /*  fast table_new_answer 3 (private leaf chaining) - use with local/batched scheduling. */
 /* do not use mode_direct_tabling.  tab.macros.h has TWO versions, one that uses the code has the hashes of afs_v04 and the second with modified code and hashes with no prev field */ 
/* ATTENTION -- added tag_as_answer_leaf_node. might cause some overhead. remove on table_new_answer for runtime testing */

/*************************************************************************
**      tries locking scheme (mandatory, define one per trie type)      **
**************************************************************************
** The (TRIE_TYPE)_LOCK_AT_ENTRY_LEVEL scheme locks the access to the   **
** table space in the entry data structure. It restricts the number of  **
** lock operations needed to go through the table data structures.      **
**                                                                      **
** The (TRIE_TYPE)_LOCK_AT_NODE_LEVEL scheme locks each data structure  **
** before accessing it. It decreases concurrrency for workers accessing **
** commom parts of the table space.                                     **
**                                                                      **
** The (TRIE_TYPE)_LOCK_AT_WRITE_LEVEL scheme is an hibrid scheme, it   **
** only locks a table data structure when it is going to update it. You **
** can use (TRIE_TYPE)_ALLOC_BEFORE_CHECK with this scheme to allocate  **
** a node before checking if it will be necessary.                      **
*************************************************************************/
//#define SUBGOAL_TRIE_LOCK_AT_ENTRY_LEVEL 1
//#define SUBGOAL_TRIE_LOCK_AT_NODE_LEVEL  1
#define SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL 1 
//#define SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS 1  
/* #define SUBGOAL_TRIE_ALLOC_BEFORE_CHECK  1 */

//#define SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL 1          /* always define this for atomic level versions - remove the write_level flag */
//#define SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V01 1
/////#define SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V03 1
//#define SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 1
//#define SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04_BUFFER_ALLOC  1  /* hash buckets only */

//#define ANSWER_TRIE_LOCK_AT_ENTRY_LEVEL 1
//#define ANSWER_TRIE_LOCK_AT_NODE_LEVEL  1
#define ANSWER_TRIE_LOCK_AT_WRITE_LEVEL 1 
//#define ANSWER_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS   1
/* #define ANSWER_TRIE_ALLOC_BEFORE_CHECK  1 */

//#define ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL 1     /* always define this for atomic level versions - remove the write_level flag */
//#define ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V01 1
//#define ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V02 1    
//////#define ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V03 1      
///#define ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 1
///#define ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04_BUFFER_ALLOC  1  /* hash buckets only */

/* #define GLOBAL_TRIE_LOCK_AT_NODE_LEVEL  1 */
#define GLOBAL_TRIE_LOCK_AT_WRITE_LEVEL 1
/* #define GLOBAL_TRIE_ALLOC_BEFORE_CHECK  1 */

/*******************************************************************
**      tries locking data structure (mandatory, define one)      **
********************************************************************
** Data structure to be used for locking the trie when using the  **
** (TRIE_TYPE)_LOCK_AT_[NODE|WRITE]_LEVEL schemes                 **
*******************************************************************/
#define TRIE_LOCK_USING_NODE_FIELD   1
//#define TRIE_LOCK_USING_GLOBAL_ARRAY 1

/*********************************************************
**      support mode directed tabling ? (optional)      **
*********************************************************/
///#define MODE_DIRECTED_TABLING 1 
//#define TIMESTAMP_MODE_DIRECTED_TABLING 1 /* must be active THREADS_SUBGOAL_SHARING + MODE_DIRECTED_TABLING --- ATTENTION : ONLY FOR INTEGERS AND MIN/MAX OPERATORS ON THE LAST ARGUMENT*/
//#define STUDY_TIMESTAMP_MDT 1


/****************************************************************
**      support early completion for tabling ? (optional)      **
*****************************************************************/
#define TABLING_EARLY_COMPLETION 1

/******************************************************
**      support trie compact pairs ? (optional)      **
******************************************************/
#define TRIE_COMPACT_PAIRS 1

/************************************************************
**      support global trie for subterms ? (optional)      **
************************************************************/
/* #define GLOBAL_TRIE_FOR_SUBTERMS 1 */

/******************************************************
**      support incomplete tabling ? (optional)      **
******************************************************/
#define INCOMPLETE_TABLING 1   /*  REMOVE IF FULL_SHARING i'm using always this simply because it is better */

/******************************************************
**      limit the table space size ? (optional)      **
******************************************************/
/* #define LIMIT_TABLING 1 */

/*********************************************************
**      support deterministic tabling ? (optional)      **
*********************************************************/
/* #define DETERMINISTIC_TABLING 1 */

/******************************************************************
**      support tabling inner cuts with OPTYap ? (optional)      **
******************************************************************/
#define TABLING_INNER_CUTS 1

/*********************************************************************
**      use timestamps for suspension with OPTYap ? (optional)      **
*********************************************************************/
#define TIMESTAMP_CHECK 1

/*************************************************
**      enable error checking ? (optional)      **
*************************************************/
/* #define DEBUG_TABLING 1 */
/* #define DEBUG_YAPOR 1 */

/**************************************************
**      extra multi-threaded tabling flags       **
**************************************************/

//DO NOT ACTIVATE THIS . NOT WORKING #define THREADS_SUBGOAL_SHARING_WITH_PAGES_SG_FR_ARRAY 1 /* enable USE_PAGES_MALLOC. problems with shmget. still missing adjusting the sg_fr back pointers to the array. when implementing, check all from the beginning*/
/* #define OUTPUT_THREADS_TABLING 1 */

//#define THREADS_LOCAL_SG_FR_HASH_BUCKETS  1 /* enable SUBGOAL_SHARING and MODE_DIRECTED flags disable THREADS_SUBGOAL_FRAME_BY_WID */
//#define THREADS_SUBGOAL_FRAME_BY_WID  1       /* enable SUBGOAL_SHARING/FULL_SHARING and MODE_DIRECTED flags disable THREADS_LOCAL_SG_FR_HASH_BUCKETS */
                           
//#define THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE  1  /* enable THREADS_SUBGOAL_FRAME_BY_WID and SUBGOAL_SHARING and MODE_DIRECTED flags disable THREADS_LOCAL_SG_FR_HASH_BUCKETS ->*/
/* -> missing to implement with THREADS_SUBGOAL_SHARING only*/

//#define THREADS_SUBGOAL_COMPLETION_WAIT  1 /* activate MODE_DIRECTED_TABLING AND SUBGOAL_SHARING. makes no sence to use this with FULL_SHARING */
                                                                                          
#if defined(THREADS_FULL_SHARING) && defined(MODE_DIRECTED_TABLING)
#ifdef USE_PAGES_MALLOC
#undef USE_PAGES_MALLOC
#endif
//#define THREADS_FULL_SHARING_MODE_DIRECTED_V01  1  /* not ok */
#define THREADS_FULL_SHARING_MODE_DIRECTED_V02  1
#endif /* THREADS_FULL_SHARING && MODE_DIRECTED_TABLING */
          

/************************************************************************
**                           Parameter Checks                          **
************************************************************************/

#ifdef YAPOR
#ifdef i386 /* For i386 machines we use shared memory segments */
#undef MMAP_MEMORY_MAPPING_SCHEME
#define SHM_MEMORY_MAPPING_SCHEME
#endif
#if !defined(MMAP_MEMORY_MAPPING_SCHEME) && !defined(SHM_MEMORY_MAPPING_SCHEME)
#error Define a memory mapping scheme
#endif
#if defined(MMAP_MEMORY_MAPPING_SCHEME) && defined(SHM_MEMORY_MAPPING_SCHEME)
#error Do not define multiple memory mapping schemes
#endif
#else /* ! YAPOR */
#undef MMAP_MEMORY_MAPPING_SCHEME
#undef SHM_MEMORY_MAPPING_SCHEME
#undef DEBUG_YAPOR
#endif /* YAPOR */

#ifdef TABLING
#if !defined(BFZ_TRAIL_SCHEME) && !defined(BBREG_TRAIL_SCHEME)
#error Define a trail scheme
#endif
#if defined(BFZ_TRAIL_SCHEME) && defined(BBREG_TRAIL_SCHEME)
#error Do not define multiple trail schemes
#endif
#else /* ! TABLING */
#undef BFZ_TRAIL_SCHEME
#undef BBREG_TRAIL_SCHEME
#undef MODE_DIRECTED_TABLING
#undef TABLING_EARLY_COMPLETION
#undef TRIE_COMPACT_PAIRS
#undef GLOBAL_TRIE_FOR_SUBTERMS
#undef INCOMPLETE_TABLING
#undef LIMIT_TABLING
#undef DETERMINISTIC_TABLING
#undef DEBUG_TABLING
#endif /* TABLING */

#if defined(TABLING) && (defined(YAPOR) || defined(THREADS))
/* SUBGOAL_TRIE_LOCK_LEVEL */
#if !defined(SUBGOAL_TRIE_LOCK_AT_ENTRY_LEVEL) && !defined(SUBGOAL_TRIE_LOCK_AT_NODE_LEVEL) && !defined(SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL) && !defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL)
#error Define a subgoal trie lock scheme
#endif
#if defined(SUBGOAL_TRIE_LOCK_AT_ENTRY_LEVEL) && defined(SUBGOAL_TRIE_LOCK_AT_NODE_LEVEL)
#error Do not define multiple subgoal trie lock schemes
#endif
#if defined(SUBGOAL_TRIE_LOCK_AT_ENTRY_LEVEL) && defined(SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL)
#error Do not define multiple subgoal trie lock schemes
#endif
#if defined(SUBGOAL_TRIE_LOCK_AT_NODE_LEVEL) && defined(SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL)
#error Do not define multiple subgoal trie lock schemes
#endif
#ifndef SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL
#undef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
#endif 
/* ANSWER_TRIE_LOCK_LEVEL */
#if !defined(ANSWER_TRIE_LOCK_AT_ENTRY_LEVEL) && !defined(ANSWER_TRIE_LOCK_AT_NODE_LEVEL) && !defined(ANSWER_TRIE_LOCK_AT_WRITE_LEVEL) && !defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL)
#error Define a answer trie lock scheme
#endif
#if defined(ANSWER_TRIE_LOCK_AT_ENTRY_LEVEL) && defined(ANSWER_TRIE_LOCK_AT_NODE_LEVEL)
#error Do not define multiple answer trie lock schemes
#endif
#if defined(ANSWER_TRIE_LOCK_AT_ENTRY_LEVEL) && defined(ANSWER_TRIE_LOCK_AT_WRITE_LEVEL)
#error Do not define multiple answer trie lock schemes
#endif
#if defined(ANSWER_TRIE_LOCK_AT_NODE_LEVEL) && defined(ANSWER_TRIE_LOCK_AT_WRITE_LEVEL)
#error Do not define multiple answer trie lock schemes
#endif
#ifndef ANSWER_TRIE_LOCK_AT_WRITE_LEVEL
#undef ANSWER_TRIE_ALLOC_BEFORE_CHECK
#endif 
/* GLOBAL_TRIE_LOCK_LEVEL */
#if !defined(GLOBAL_TRIE_LOCK_AT_NODE_LEVEL) && !defined(GLOBAL_TRIE_LOCK_AT_WRITE_LEVEL)
#error Define a global trie lock scheme
#endif
#if defined(GLOBAL_TRIE_LOCK_AT_NODE_LEVEL) && defined(GLOBAL_TRIE_LOCK_AT_WRITE_LEVEL)
#error Do not define multiple global trie lock schemes
#endif
#ifndef GLOBAL_TRIE_LOCK_AT_WRITE_LEVEL
#undef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
#endif
/* TRIE_LOCK_USING */
#if !defined(TRIE_LOCK_USING_NODE_FIELD) && !defined(TRIE_LOCK_USING_GLOBAL_ARRAY)
#error Define a trie lock data structure
#endif
#if defined(TRIE_LOCK_USING_NODE_FIELD) && defined(TRIE_LOCK_USING_GLOBAL_ARRAY)
#error Do not define multiple trie lock data structures
#endif
#else /* ! TABLING || (! YAPOR && ! THREADS) */
#undef SUBGOAL_TRIE_LOCK_AT_ENTRY_LEVEL
#undef SUBGOAL_TRIE_LOCK_AT_NODE_LEVEL
#undef SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL
#undef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
#undef ANSWER_TRIE_LOCK_AT_ENTRY_LEVEL
#undef ANSWER_TRIE_LOCK_AT_NODE_LEVEL
#undef ANSWER_TRIE_LOCK_AT_WRITE_LEVEL
#undef ANSWER_TRIE_ALLOC_BEFORE_CHECK
#undef GLOBAL_TRIE_LOCK_AT_NODE_LEVEL
#undef GLOBAL_TRIE_LOCK_AT_WRITE_LEVEL
#undef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
#undef TRIE_LOCK_USING_NODE_FIELD
#undef TRIE_LOCK_USING_GLOBAL_ARRAY
#endif /* TABLING && (YAPOR || THREADS) */

#if defined(TABLING) && defined(THREADS)
#if !defined(THREADS_NO_SHARING) && !defined(THREADS_SUBGOAL_SHARING) && !defined(THREADS_FULL_SHARING)
#error Define a multithreading table design
#endif
#if defined(THREADS_NO_SHARING) && defined(THREADS_SUBGOAL_SHARING)
#error Do not define multiple multithreading table designs
#endif
#if defined(THREADS_NO_SHARING) && defined(THREADS_FULL_SHARING)
#error Do not define multiple multithreading table designs
#endif
#if defined(THREADS_SUBGOAL_SHARING) && defined(THREADS_FULL_SHARING)
#error Do not define multiple multithreading table designs
#endif

#ifdef THREADS_NO_SHARING
#undef SUBGOAL_TRIE_LOCK_AT_ENTRY_LEVEL
#undef SUBGOAL_TRIE_LOCK_AT_NODE_LEVEL
#undef SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL
#undef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
#undef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL
#undef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V01
#undef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V03
#endif
#if defined(THREADS_NO_SHARING) || defined(THREADS_SUBGOAL_SHARING)
#undef ANSWER_TRIE_LOCK_AT_ENTRY_LEVEL
#undef ANSWER_TRIE_LOCK_AT_NODE_LEVEL
#undef ANSWER_TRIE_LOCK_AT_WRITE_LEVEL
#undef ANSWER_TRIE_ALLOC_BEFORE_CHECK
#undef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL
#undef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V01
#undef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V02
#undef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V03

#endif
#else /* ! TABLING || ! THREADS */
#undef THREADS_NO_SHARING
#undef THREADS_SUBGOAL_SHARING
#undef THREADS_FULL_SHARING
#endif /* TABLING && THREADS */

#ifdef TRIE_LOCK_USING_NODE_FIELD
#if defined(SUBGOAL_TRIE_LOCK_AT_NODE_LEVEL) || defined(SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL)
#define SUBGOAL_TRIE_LOCK_USING_NODE_FIELD   1
#endif
#if defined(ANSWER_TRIE_LOCK_AT_NODE_LEVEL) || defined(ANSWER_TRIE_LOCK_AT_WRITE_LEVEL)
#define ANSWER_TRIE_LOCK_USING_NODE_FIELD    1
#endif
#if defined(GLOBAL_TRIE_LOCK_AT_NODE_LEVEL) || defined(GLOBAL_TRIE_LOCK_AT_WRITE_LEVEL)
#define GLOBAL_TRIE_LOCK_USING_NODE_FIELD    1
#endif
#elif defined(TRIE_LOCK_USING_GLOBAL_ARRAY)
#if defined(SUBGOAL_TRIE_LOCK_AT_NODE_LEVEL) || defined(SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL)
#define SUBGOAL_TRIE_LOCK_USING_GLOBAL_ARRAY 1
#endif
#if defined(ANSWER_TRIE_LOCK_AT_NODE_LEVEL) || defined(ANSWER_TRIE_LOCK_AT_WRITE_LEVEL)
#define ANSWER_TRIE_LOCK_USING_GLOBAL_ARRAY  1
#endif
#if defined(GLOBAL_TRIE_LOCK_AT_NODE_LEVEL) || defined(GLOBAL_TRIE_LOCK_AT_WRITE_LEVEL)
#define GLOBAL_TRIE_LOCK_USING_GLOBAL_ARRAY  1
#endif
#endif

#if !defined(TABLING) || !defined(YAPOR)
#undef TABLING_INNER_CUTS
#undef TIMESTAMP_CHECK
#endif

#if !defined(TABLING) || !defined(THREADS)
#undef OUTPUT_THREADS_TABLING
#endif

#if defined(DEBUG_YAPOR) && defined(DEBUG_TABLING)
#define DEBUG_OPTYAP
#endif

#if defined(LIMIT_TABLING) && !defined(USE_PAGES_MALLOC)
#error LIMIT_TABLING requires USE_PAGES_MALLOC
#endif


#if defined(YAPOR) /*|| defined(THREADS_FULL_SHARING) */
#undef TABLING_EARLY_COMPLETION
#endif


#if defined(YAPOR) || defined(THREADS)
//#undef INCOMPLETE_TABLING
#undef LIMIT_TABLING
#undef DETERMINISTIC_TABLING
#endif

#ifndef THREADS_FULL_SHARING
#ifdef THREADS_FULL_SHARING_FTNA
#undef THREADS_FULL_SHARING_FTNA
#endif /* THREADS_FULL_SHARING_FTNA */
#endif /* THREADS_FULL_SHARING */
          


/****************************************** YAP ISSUES ***********************/
/* CHECK miguel tag on C/index.c file */



#ifdef TIMESTAMP_MODE_DIRECTED_TABLING

#else /* !TIMESTAMP_MODE_DIRECTED_TABLING */

#endif /* TIMESTAMP_MODE_DIRECTED_TABLING */
