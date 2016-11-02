#include <stdio.h>
#include <math.h>

/* structure definitions */
typedef struct cache_line_ {
  unsigned tag;
  int dirty;

  struct cache_line_ *LRU_next;
  struct cache_line_ *LRU_prev;
} cache_line, *Pcache_line;

typedef struct cache_ {
  int size;			/* cache size */
  int associativity;		/* cache associativity */
  int n_sets;			/* number of cache sets */
  unsigned index_mask;		/* mask to find cache index */
  int index_mask_offset;	/* number of zero bits in mask */
  Pcache_line *LRU_head;	/* head of LRU list for each set */
  Pcache_line *LRU_tail;	/* tail of LRU list for each set */
  int *set_contents;		/* number of valid entries in set */
} cache, *Pcache;

/* cache model data structures */
static Pcache icache;
static Pcache dcache;
static cache c1;
static cache c2;
static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;

int main(){
   /*Allocate an arrary of cache line pointer*/
  icache->LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*icache->n_sets);
  for (int i = 0; i < icache->n_sets; i++) {
    icache->LRU_head[i] = NULL;
  }

  return 0;
}