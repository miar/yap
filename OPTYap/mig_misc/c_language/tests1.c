#include <stdio.h>

#define T1
#include "tests1.i"
#undef T1


#define T2
#include "tests1.i"
#undef T2



int main() {
  int ar1=3, a=0,b=0;
  subgoal_trie(ar1, a, b);
  answer_trie(ar1);
  return 0;
}


