/*************************************************************************
*									 *
*	 YAP Prolog 	%W% %G%		*
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		clause.h						 *
* Last rev:								 *
* mods:									 *
* comments:	clause info						 *
*									 *
*************************************************************************/

#include "Yatom.h"
#include "Heap.h"

/* consulting files */

typedef union CONSULT_OBJ {
  char *filename;
  int mode;
  Prop  p;
  UInt c;
} consult_obj;

/* Either we are assembling clauses or indexing code */

#define ASSEMBLING_CLAUSE	0
#define ASSEMBLING_INDEX	1
#define ASSEMBLING_EINDEX	2

#define NextDynamicClause(X)	(((yamop *)X)->u.ld.d)

#define PredFirstClause		0
#define PredMiddleClause	1
#define PredLastClause		2

typedef struct logic_upd_index {
  CELL             ClFlags;
  UInt             ClRefCount;
#if defined(YAPOR) || defined(THREADS)
  /* A lock for manipulating the clause */
  lockvar          ClLock;
#endif
  UInt		   ClSize;
  union {
    PredEntry *pred;
    struct logic_upd_index *ParentIndex;
  } u;
  struct logic_upd_index *SiblingIndex;
  struct logic_upd_index *ChildIndex;
  /* The instructions, at least one of the form sl */
  yamop            ClCode[MIN_ARRAY];
} LogUpdIndex;

/* The ordering of the first 3 fields should be compatible with dbrefs */
typedef struct logic_upd_clause {
  Functor Id;		/* allow pointers to this struct to id  */
			/*   as dbref                           */
  /* A set of flags describing info on the clause */
  /* A set of flags describing info on the clause */
  CELL             ClFlags;
#if defined(YAPOR) || defined(THREADS)
  /* A lock for manipulating the clause */
  lockvar          ClLock;
#endif
  UInt		  ClSize;
  /* extra clause information for logical update indices and facts */
  /* indices that may still backtrack to this clause */
  UInt             ClRefCount;
  /* data for clauses  with environments */
  yamop           *ClExt;
  DBTerm          *ClSource;
  /* doubly linked list of clauses */
  struct logic_upd_clause   *ClPrev, *ClNext;
  /* parent pointer */
  PredEntry   *ClPred;
  /* The instructions, at least one of the form sl */
  yamop            ClCode[MIN_ARRAY];
} LogUpdClause;

typedef struct dynamic_clause {
  /* A set of flags describing info on the clause */
  CELL            ClFlags;
#if defined(YAPOR) || defined(THREADS)
  /* A lock for manipulating the clause */
  lockvar          ClLock;
#endif
  UInt		   ClSize;
  UInt             ClRefCount;
  yamop              *ClPrevious;     /* immediate update clause */
  /* The instructions, at least one of the form sl */
  yamop            ClCode[MIN_ARRAY];
} DynamicClause;

typedef struct static_index {
  /* A set of flags describing info on the clause */
  CELL            ClFlags;
  UInt		  ClSize;
  struct static_index *SiblingIndex;
  struct static_index *ChildIndex;
  /* The instructions, at least one of the form sl */
  yamop            ClCode[MIN_ARRAY];
} StaticIndex;

typedef struct static_clause {
  /* A set of flags describing info on the clause */
  CELL            ClFlags;
  UInt		  ClSize;
  union {
    DBTerm          *ClSource;
    PredEntry       *ClPred;
  } usc;
  struct static_clause   *ClNext;
  /* The instructions, at least one of the form sl */
  yamop            ClCode[MIN_ARRAY];
} StaticClause;

typedef struct static_mega_clause {
  /* A set of flags describing info on the clause */
  CELL            ClFlags;
  UInt		  ClSize;
  PredEntry      *ClPred;
  UInt            ClItemSize;
  /* The instructions, at least one of the form sl */
  yamop            ClCode[MIN_ARRAY];
} MegaClause;

typedef struct dead_clause {
  CELL            ClFlags;
#if defined(YAPOR) || defined(THREADS)
  /* A lock for manipulating the clause */
  lockvar          ClLock;
  UInt             ref_count;
#endif
  UInt ClSize;
  struct dead_clause *NextCl;       /* dead clause */
} DeadClause;

typedef union clause_obj {
  struct logic_upd_clause luc;
  struct logic_upd_index lui;
  struct dynamic_clause ic;
  struct static_clause sc;
  struct static_mega_clause mc;
  struct static_index si;
} ClauseUnion;

#define ClauseCodeToDynamicClause(p)    ((DynamicClause *)((CODEADDR)(p)-(CELL)(((DynamicClause *)NULL)->ClCode)))
#define ClauseCodeToStaticClause(p)    ((StaticClause *)((CODEADDR)(p)-(CELL)(((StaticClause *)NULL)->ClCode)))
#define ClauseCodeToLogUpdClause(p)    ((LogUpdClause *)((CODEADDR)(p)-(CELL)(((LogUpdClause *)NULL)->ClCode)))
#define ClauseCodeToMegaClause(p)    ((MegaClause *)((CODEADDR)(p)-(CELL)(((MegaClause *)NULL)->ClCode)))
#define ClauseCodeToLogUpdIndex(p)    ((LogUpdIndex *)((CODEADDR)(p)-(CELL)(((LogUpdIndex *)NULL)->ClCode)))
#define ClauseCodeToStaticIndex(p)    ((StaticIndex *)((CODEADDR)(p)-(CELL)(((StaticIndex *)NULL)->ClCode)))

#define ClauseFlagsToDynamicClause(p)    ((DynamicClause *)(p))
#define ClauseFlagsToLogUpdClause(p)     ((LogUpdClause *)((CODEADDR)(p)-(CELL)(&(((LogUpdClause *)NULL)->ClFlags))))
#define ClauseFlagsToLogUpdIndex(p)      ((LogUpdIndex *)((CODEADDR)(p)-(CELL)(&(((LogUpdIndex *)NULL)->ClFlags))))
#define ClauseFlagsToStaticClause(p)     ((StaticClause *)(p))

#define DynamicFlags(X)		(ClauseCodeToDynamicClause(X)->ClFlags)

#define DynamicLock(X)		(ClauseCodeToDynamicClause(X)->ClLock)

#if defined(YAPOR) || defined(THREADS)
#define INIT_CLREF_COUNT(X) (X)->ClRefCount = 0
#define  INC_CLREF_COUNT(X) (X)->ClRefCount++
#define  DEC_CLREF_COUNT(X) (X)->ClRefCount--
#define        CL_IN_USE(X) ((X)->ClRefCount)
#else
#define INIT_CLREF_COUNT(X)
#define  INC_CLREF_COUNT(X) 
#define  DEC_CLREF_COUNT(X) 
#define        CL_IN_USE(X) ((X)->ClFlags & InUseMask || (X)->ClRefCount)
#endif

/* amasm.c */
wamreg	STD_PROTO(Yap_emit_x,(CELL));
wamreg  STD_PROTO(Yap_compile_cmp_flags,(PredEntry *));
void    STD_PROTO(Yap_InitComma,(void));

/* cdmgr.c */
void	STD_PROTO(Yap_IPred,(PredEntry *));
void	STD_PROTO(Yap_addclause,(Term,yamop *,int,Term));
void	STD_PROTO(Yap_add_logupd_clause,(PredEntry *,LogUpdClause *,int));
void	STD_PROTO(Yap_kill_iblock,(ClauseUnion *,ClauseUnion *,PredEntry *));
void	STD_PROTO(Yap_cleanup_dangling_indices,(yamop *,yamop *,yamop *,yamop *));
void	STD_PROTO(Yap_EraseStaticClause,(StaticClause *, Term));
ClauseUnion *STD_PROTO(Yap_find_owner_index,(yamop *, PredEntry *));

/* dbase.c */
void	STD_PROTO(Yap_ErCl,(DynamicClause *));
void	STD_PROTO(Yap_ErLogUpdCl,(LogUpdClause *));
void	STD_PROTO(Yap_ErLogUpdIndex,(LogUpdIndex *));

/* exec.c */
Term    STD_PROTO(Yap_cp_as_integer,(choiceptr));

/* index.c */
yamop   *STD_PROTO(Yap_PredIsIndexable,(PredEntry *));
yamop   *STD_PROTO(Yap_ExpandIndex,(PredEntry *));
yamop   *STD_PROTO(Yap_CleanUpIndex,(struct logic_upd_index *));
void     STD_PROTO(Yap_AddClauseToIndex,(PredEntry *,yamop *,int));
void     STD_PROTO(Yap_RemoveClauseFromIndex,(PredEntry *,yamop *));
LogUpdClause  *STD_PROTO(Yap_NthClause,(PredEntry *,Int));
LogUpdClause  *STD_PROTO(Yap_FollowIndexingCode,(PredEntry *,yamop *, Term *, yamop *,yamop *));

#if USE_THREADED_CODE

#define OP_HASH_SIZE 2048

static inline int
rtable_hash_op(OPCODE opc, int hash_mask) {
  return((((CELL)opc) >> 3) & hash_mask);
}

/* given an opcode find the corresponding opnumber. This should make
   switches on ops a much easier operation */
static inline op_numbers
Yap_op_from_opcode(OPCODE opc)
{
  int j = rtable_hash_op(opc,OP_HASH_SIZE-1);

  while (OP_RTABLE[j].opc != opc) {
    if (!OP_RTABLE[j].opc)
      return _Nstop;
    if (j == OP_HASH_SIZE-1) {
      j = 0;
    } else {
      j++;
    }
  }
  return OP_RTABLE[j].opnum;
}
#else
static inline op_numbers
Yap_op_from_opcode(OPCODE opc)
{
  return((op_numbers)opc);
}
#endif /* USE_THREADED_CODE */

#if defined(YAPOR) || defined(THREADS)
static int
same_lu_block(yamop **paddr, yamop *p)
{
  yamop *np = *paddr;
  if (np != p) {
    OPCODE jmp_op = Yap_opcode(_jump_if_nonvar);

    while (np->opc == jmp_op) {
      np = NEXTOP(np, xl);
      if (np == p) return TRUE;
    }
    return FALSE;
  } else {
    return TRUE;
  }
}
#endif

static inline Term 
Yap_MkStaticRefTerm(StaticClause *cp)
{
  Term t[1];
  t[0] = MkIntegerTerm((Int)cp);
  return Yap_MkApplTerm(FunctorStaticClause,1,t);
}

static inline StaticClause *
Yap_ClauseFromTerm(Term t)
{
  return (StaticClause *)IntegerOfTerm(ArgOfTerm(1,t));
}

static inline Term 
Yap_MkMegaRefTerm(PredEntry *ap,yamop *ipc)
{
  Term t[2];
  t[0] = MkIntegerTerm((Int)ap);
  t[0] = MkIntegerTerm((Int)ipc);
  return Yap_MkApplTerm(FunctorMegaClause,2,t);
}

static inline yamop * 
Yap_MegaClauseFromTerm(Term t)
{
  return (yamop *)IntegerOfTerm(ArgOfTerm(1,t));
}

static inline PredEntry * 
Yap_MegaClausePredicateFromTerm(Term t)
{
  return (PredEntry *)IntegerOfTerm(ArgOfTerm(2,t));
}

