#ifdef T1
#define USES_ARGS , int a, int b
#define PASS_ARGS , a, b

#define FUNC          subgoal_trie
#define CALL_FUNC(X)  subgoal_trie(X PASS_ARGS)
static inline int FUNC (int ar1 USES_ARGS );
#endif

#ifdef T2
#define USES_ARGS
#define PASS_ARGS
#define FUNC          long answer_trie
#define CALL_FUNC(X)  answer_trie(X PASS_ARGS)
static inline int FUNC (int USES_ARGS);
#endif

static inline int FUNC (int ar1 USES_ARGS) {
  if (ar1 == 0)
    return 0;
  printf("ar1 = %d\n", ar1);
  return CALL_FUNC(ar1 - 1);
}


#undef FUNC
#undef CALL_FUNC
#undef USES_ARGS
#undef PASS_ARGS
