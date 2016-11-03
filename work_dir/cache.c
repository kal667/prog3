/*
 * 
 * cache.c
 * 
 * Donald Yeung
 */


#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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

  printf("Initializing cache\n");
  int i;

  /*initialize cache data structure*/
    
    if (cache_split == FALSE) {
        /*cache size in words*/
        c1.size = cache_usize / WORD_SIZE;
        /*cache associativity*/
        c1.associativity = cache_assoc; /*For direct mapped*/
        /*number of cache sets*/
        c1.n_sets = cache_usize / cache_block_size;
        /*mask to find cache index*/
        c1.index_mask = (cache_usize / cache_block_size - 1) << LOG2(cache_block_size);
        /*number of zero bits in mask*/
        c1.index_mask_offset = LOG2(cache_block_size);
        /*number of valid entries in set*/
        c1.set_contents = &cache_assoc; /*for direct mapped*/
        /*Allocate an array of cache line pointer*/
        c1.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets);
  
        for (i = 0; i < c1.n_sets; i++) {
        c1.LRU_head[i] = NULL;
        }
    }

    if (cache_split ==TRUE) {
        
        /*Instruction cache*/
        /*cache size in words*/
        c1.size = cache_isize / WORD_SIZE;
        /*cache associativity*/
        c1.associativity = cache_assoc; /*For direct mapped*/
        /*number of cache sets*/
        c1.n_sets = cache_isize / cache_block_size;
        /*mask to find cache index*/
        c1.index_mask = (cache_isize / cache_block_size - 1) << LOG2(cache_block_size);
        /*number of zero bits in mask*/
        c1.index_mask_offset = LOG2(cache_block_size);
        /*number of valid entries in set*/
        c1.set_contents = &cache_assoc; /*for direct mapped*/
        /*Allocate an array of cache line pointer*/
        c1.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets);
 
        for (i = 0; i < c1.n_sets; i++) {
        c1.LRU_head[i] = NULL;
        }

        /*Data Cache*/
        /*cache size in words*/
        c2.size = cache_dsize / WORD_SIZE;
        /*cache associativity*/
        c2.associativity = cache_assoc; /*For direct mapped*/
        /*number of cache sets*/
        c2.n_sets = cache_dsize / cache_block_size;
        /*mask to find cache index*/
        c2.index_mask = (cache_dsize / cache_block_size - 1) << LOG2(cache_block_size);
        /*number of zero bits in mask*/
        c2.index_mask_offset = LOG2(cache_block_size);
        /*number of valid entries in set*/
        c2.set_contents = &cache_assoc; /*for direct mapped*/
        /*Allocate an array of cache line pointer*/
        c2.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c2.n_sets);  
        
        for (i = 0; i < c2.n_sets; i++) {
        c2.LRU_head[i] = NULL;
        }
    }
  
  /*initialize cache statistics data structure*/
  printf("Initializing cache stats\n");
  cache_stat cache_stat_inst = {0};
  cache_stat cache_stat_data = {0};
  printf("Done Initializing cache stats\n");

}
/************************************************************/

/************************************************************/
void perform_access(addr, access_type)
  unsigned addr, access_type;
{

    /* handle an access to the cache */

/*
    printf("In perform access\n");
    printf("access type (%u)\n", access_type);
    printf("address (0x%x)\n", addr);
    printf("index (%d) of (%d)\n", index, c1.n_sets);
    printf("addr_tag (0x%x)\n", addr_tag);
*/

    unsigned index = (addr & c1.index_mask) >> c1.index_mask_offset;
    unsigned addr_tag = addr >> (c1.index_mask_offset + LOG2(c1.n_sets));

    /*data load*/
    if (access_type == 0){
        /*printf("Data Load\n");*/
        cache_stat_data.accesses += 1;

        if (cache_split == FALSE) {
            /*cache miss on NULL*/
            if (c1.LRU_head[index] == NULL){
                c1.LRU_head[index] = (Pcache_line *)malloc(sizeof(cache_line));
                c1.LRU_head[index]->tag = addr_tag;
                cache_stat_data.misses += 1;
            }
            
            /*cache hit*/
            else if (c1.LRU_head[index]->tag == addr_tag) {

            }
            
            /*cache miss*/
            else {
                c1.LRU_head[index]->tag = addr_tag;
                cache_stat_data.misses += 1;
                cache_stat_data.replacements += 1;
            }
        }

        else {
            /*cache miss on NULL*/
            if (c2.LRU_head[index] == NULL){
                c2.LRU_head[index] = (Pcache_line *)malloc(sizeof(cache_line));
                c2.LRU_head[index]->tag = addr_tag;
                cache_stat_data.misses += 1;
            }
            
            /*cache hit*/
            else if (c2.LRU_head[index]->tag == addr_tag) {

            }
            
            /*cache miss*/
            else {
                c2.LRU_head[index]->tag = addr_tag;
                cache_stat_data.misses += 1;
                cache_stat_data.replacements += 1;
            }
        }
    }

    /*data store*/
    if (access_type == 1) {
        /*printf("Data store\n");*/
        cache_stat_data.accesses += 1;
        
        if (cache_split == FALSE) {
            /*cache miss on NULL*/
            if (c1.LRU_head[index] == NULL){
                c1.LRU_head[index] = (Pcache_line *)malloc(sizeof(cache_line));
                c1.LRU_head[index]->tag = addr_tag;
                cache_stat_data.misses += 1;
            }
            
            /*cache hit*/
            else if (c1.LRU_head[index]->tag == addr_tag) {

            }
            
            /*cache miss*/
            else {
                c1.LRU_head[index]->tag = addr_tag;
                cache_stat_data.misses += 1;
                cache_stat_data.replacements += 1;
            }
        }

        else {
            /*cache miss on NULL*/
            if (c2.LRU_head[index] == NULL){
                c2.LRU_head[index] = (Pcache_line *)malloc(sizeof(cache_line));
                c2.LRU_head[index]->tag = addr_tag;
                cache_stat_data.misses += 1;
            }
            
            /*cache hit*/
            else if (c2.LRU_head[index]->tag == addr_tag) {

            }
            
            /*cache miss*/
            else {
                c2.LRU_head[index]->tag = addr_tag;
                cache_stat_data.misses += 1;
                cache_stat_data.replacements += 1;
            }
        }
    }

    /*instruction load*/
    if (access_type == 2) {
        /*printf("Instruction Load\n");*/
        cache_stat_inst.accesses += 1;
        
        /*cache miss on NULL*/
        if (c1.LRU_head[index] == NULL) {
            c1.LRU_head[index] = (Pcache_line *)malloc(sizeof(cache_line));
            c1.LRU_head[index]->tag = addr_tag;
            cache_stat_inst.misses += 1;
        }
        
        /*cache hit*/
        else if (c1.LRU_head[index]->tag == addr_tag) {
        }
        
        /*cache miss*/
        else {
            c1.LRU_head[index]->tag = addr_tag;
            cache_stat_inst.misses += 1;
            cache_stat_inst.replacements += 1;
        }
    }

    return;
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