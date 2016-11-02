/*
 * 
 * cache.c
 * 
 * Donald Yeung
 */


#include <stdio.h>
#include <math.h>

#include "cache.h"
#include "main.h"

/* cache configuration parameters */
static int cache_split = 0;
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_isize = DEFAULT_CACHE_SIZE; 
static int cache_dsize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;

/* cache model data structures */
static Pcache icache;
static Pcache dcache;
static cache c1;
static cache c2;
static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;

/************************************************************/
void set_cache_param(param, value)
  int param;
  int value;
{

  switch (param) {
  case CACHE_PARAM_BLOCK_SIZE:
    cache_block_size = value;
    words_per_block = value / WORD_SIZE;
    break;
  case CACHE_PARAM_USIZE:
    cache_split = FALSE;
    cache_usize = value;
    break;
  case CACHE_PARAM_ISIZE:
    cache_split = TRUE;
    cache_isize = value;
    break;
  case CACHE_PARAM_DSIZE:
    cache_split = TRUE;
    cache_dsize = value;
    break;
  case CACHE_PARAM_ASSOC:
    cache_assoc = value;
    break;
  case CACHE_PARAM_WRITEBACK:
    cache_writeback = TRUE;
    break;
  case CACHE_PARAM_WRITETHROUGH:
    cache_writeback = FALSE;
    break;
  case CACHE_PARAM_WRITEALLOC:
    cache_writealloc = TRUE;
    break;
  case CACHE_PARAM_NOWRITEALLOC:
    cache_writealloc = FALSE;
    break;
  default:
    printf("error set_cache_param: bad parameter value\n");
    exit(-1);
  }

}
/************************************************************/

/************************************************************/
void init_cache()
{

  /* initialize the cache, and cache statistics data structures */

  /*initialize cache data structure*/
  /*cache size in words*/
  c1.size = DEFAULT_CACHE_SIZE / WORD_SIZE;
  /*cache associativity*/
  c1.associativity = DEFAULT_CACHE_ASSOC; /*For direct mapped*/
  /*number of cache sets*/
  c1.n_sets = DEFAULT_CACHE_SIZE / DEFAULT_CACHE_BLOCK_SIZE;
  /*mask to find cache index*/
  c1.index_mask = LOG2(DEFAULT_CACHE_SIZE / DEFAULT_CACHE_BLOCK_SIZE) - 1;
  /*number of zero bits in mask*/
  c1.index_mask_offset = LOG2(DEFAULT_CACHE_BLOCK_SIZE);
  /*number of valid entries in set*/
  c1.set_contents = 1; /*for direct mapped*/

  /*Allocate an arrary of cache line pointer*/
  c1.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c1->n_sets);
  /*
  TODO: initialize each entry to NULL
  */
  int i;
  for (i = 0; i < c1.n_sets; i++){
    c1.LRU_head[i] = NULL;
  }

  /*initialize cache statistics data structure*/
  
  cache_stat_inst = NULL;
  cache_stat_data = NULL;

}
/************************************************************/

/************************************************************/
void perform_access(addr, access_type)
  unsigned addr, access_type;
{

    /* handle an access to the cache */

    unsigned index = (addr & c1.index_mask) >> c1.index_mask_offset;

    
    /*data load*/
    if (access_type = TRACE_DATA_LOAD){
        /*cache hit*/
        if (c1.LRU_head[index].tag == addr) {

        }
        /*cache miss on NULL*/
        else if (c1.LRU_head[index].tag == NULL){
            c1.LRU_head[index].tag = addr;
            cache_stat_data.misses += 1;
        }
        /*cache miss*/
        else {
            c1.LRU_head[index].tag = addr;
            cache_stat_data.misses += 1;
        }
    }

    /*data store*/
    if (access_type = TRACE_DATA_STORE) {
        /*cache hit*/
        if (c1.LRU_head[index].tag == addr) {

        }
        /*cache miss on NULL*/
        else if (c1.LRU_head[index].tag == NULL) {
            c1.LRU_head[index].tag = addr;
            cache_stat_data.misses += 1;
        }
        /*cache miss*/
        else {
            c1.LRU_head[index].tag = addr;
            cache_stat_data.misses += 1;
            cache_stat_data.replacements += 1;
        }
    }

    /*instruction load*/
    if (access_type = TRACE_INST_LOAD) {
        /*cache hit*/
        if (c1.LRU_head[index].tag == addr) {

        }
        /*cache miss on NULL*/
        else if (c1.LRU_head[index].tag == NULL) {
            c1.LRU_head[index].tag = addr;
            cache_stat_inst.misses += 1;
        }
        /*cache miss*/
        else {
            c1.LRU_head[index].tag = addr;
            cache_stat_inst.misses += 1;
            cache_stat_inst.replacements += 1;
        }
    }

}
/************************************************************/

/************************************************************/
void flush()
{

  /* flush the cache */

    int i;
    for (i = 0; i < c1.n_sets; i++){
    c1.LRU_head[i] = NULL;
    }

}
/************************************************************/

/************************************************************/
void delete(head, tail, item)
  Pcache_line *head, *tail;
  Pcache_line item;
{
  if (item->LRU_prev) {
    item->LRU_prev->LRU_next = item->LRU_next;
  } else {
    /* item at head */
    *head = item->LRU_next;
  }

  if (item->LRU_next) {
    item->LRU_next->LRU_prev = item->LRU_prev;
  } else {
    /* item at tail */
    *tail = item->LRU_prev;
  }
}
/************************************************************/

/************************************************************/
/* inserts at the head of the list */
void insert(head, tail, item)
  Pcache_line *head, *tail;
  Pcache_line item;
{
  item->LRU_next = *head;
  item->LRU_prev = (Pcache_line)NULL;

  if (item->LRU_next)
    item->LRU_next->LRU_prev = item;
  else
    *tail = item;

  *head = item;
}
/************************************************************/

/************************************************************/
void dump_settings()
{
  printf("Cache Settings:\n");
  if (cache_split) {
    printf("\tSplit I- D-cache\n");
    printf("\tI-cache size: \t%d\n", cache_isize);
    printf("\tD-cache size: \t%d\n", cache_dsize);
  } else {
    printf("\tUnified I- D-cache\n");
    printf("\tSize: \t%d\n", cache_usize);
  }
  printf("\tAssociativity: \t%d\n", cache_assoc);
  printf("\tBlock size: \t%d\n", cache_block_size);
  printf("\tWrite policy: \t%s\n", 
	 cache_writeback ? "WRITE BACK" : "WRITE THROUGH");
  printf("\tAllocation policy: \t%s\n",
	 cache_writealloc ? "WRITE ALLOCATE" : "WRITE NO ALLOCATE");
}
/************************************************************/

/************************************************************/
void print_stats()
{
  printf("*** CACHE STATISTICS ***\n");
  printf("  INSTRUCTIONS\n");
  printf("  accesses:  %d\n", cache_stat_inst.accesses);
  printf("  misses:    %d\n", cache_stat_inst.misses);
  printf("  miss rate: %f\n", 
	 (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses);
  printf("  replace:   %d\n", cache_stat_inst.replacements);

  printf("  DATA\n");
  printf("  accesses:  %d\n", cache_stat_data.accesses);
  printf("  misses:    %d\n", cache_stat_data.misses);
  printf("  miss rate: %f\n", 
	 (float)cache_stat_data.misses / (float)cache_stat_data.accesses);
  printf("  replace:   %d\n", cache_stat_data.replacements);

  printf("  TRAFFIC (in words)\n");
  printf("  demand fetch:  %d\n", cache_stat_inst.demand_fetches + 
	 cache_stat_data.demand_fetches);
  printf("  copies back:   %d\n", cache_stat_inst.copies_back +
	 cache_stat_data.copies_back);
}
/************************************************************/