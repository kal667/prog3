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
    //unified cache
    if(cache_split == 0) {
        c1.size = cache_usize/4;    
        c1.associativity = cache_assoc;
        c1.n_sets = cache_usize/cache_block_size/cache_assoc;
        c1.index_mask = (c1.n_sets-1) << LOG2(cache_block_size);
        c1.index_mask_offset = LOG2(cache_block_size);  
        
        //allocate the array of cache line pointers
        c1.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets);
        c1.LRU_tail = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets);
        c1.set_contents = (int *)malloc(sizeof(int)*c1.n_sets);
        int i;
        //allocate each cache line in the pointer array
        for(i = 0; i < c1.n_sets; i++) {
            c1.LRU_head[i] = NULL;
            c1.LRU_tail[i] = NULL;
            c1.set_contents[i] = 0;
        }
    }
    //split cache
    else {
        //I-cache initialization
        c1.size = cache_isize/4;
        c1.associativity = cache_assoc;
        c1.n_sets = cache_isize/cache_block_size/cache_assoc;
        c1.index_mask = (c1.n_sets-1) << LOG2(cache_block_size);
        c1.index_mask_offset = LOG2(cache_block_size);
        
        //allocate the array of cache line pointers
        c1.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets);
        c1.LRU_tail = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets);
        c1.set_contents = (int *)malloc(sizeof(int)*c1.n_sets);
        int i;
        //allocate each cache line in the pointer array
        for(i = 0; i < c1.n_sets; i++) {
            c1.LRU_head[i] = NULL;
            c1.LRU_tail[i] = NULL;
            c1.set_contents[i] = 0;
        }
        
        //D-cache initialization
        c2.size = cache_dsize/4;
        c2.associativity = cache_assoc;
        c2.n_sets = cache_dsize/cache_block_size/cache_assoc;
        c2.index_mask = (c2.n_sets-1) << LOG2(cache_block_size);
        c2.index_mask_offset = LOG2(cache_block_size);
        
        //allocate the array of cache line pointers
        c2.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c2.n_sets);
        c2.LRU_tail = (Pcache_line *)malloc(sizeof(Pcache_line)*c2.n_sets);
        c2.set_contents = (int *)malloc(sizeof(int)*c2.n_sets);
        //allocate each cache line in the pointer array
        for(i = 0; i < c2.n_sets; i++) {
            c2.LRU_head[i] = NULL;
            c2.LRU_tail[i] = NULL;
            c2.set_contents[i] = 0;
        }
    }
}
/************************************************************/

/************************************************************/
void perform_access(addr, access_type)
  unsigned addr, access_type;
{

    /* handle an access to the cache */

    unsigned index = (addr & c1.index_mask) >> c1.index_mask_offset;
    unsigned addr_tag = addr >> (c1.index_mask_offset + LOG2(c1.n_sets));

    int i;
    int hit_flag;
    Pcache_line temp;


    printf("In perform access\n");
    printf("access type (%u)\n", access_type);
    printf("address (0x%x)\n", addr);
    printf("index (%d) of (%d)\n", index, c1.n_sets);
    printf("addr_tag (0x%x)\n", addr_tag);


    /*data load*/
    if (access_type == 0){

        hit_flag = FALSE;

        printf("Data Load\n");
        cache_stat_data.accesses += 1;

        /*Unified cache*/
        if (cache_split == FALSE) {
            /*cache miss on NULL*/
            
            printf("LRU_head[index] = 0x%x\n", c1.LRU_head[index]);
            printf("c1.LRU_head[index] 0x%x\n", c1.LRU_head[index]);
        
            /*cache miss on NULL*/
            if (c1.LRU_head[index] == NULL) {
                printf("Cache miss on NULL\n");

                temp = (Pcache_line *)malloc(sizeof(cache_line));
                temp->tag = addr_tag;
                temp->dirty = FALSE;
                temp->LRU_prev = NULL;
                temp->LRU_next = NULL;

                c1.LRU_head[index] = temp;
                c1.LRU_tail[index] = temp;

                cache_stat_data.misses += 1;
                cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                c1.set_contents[index] += 1;
            }
            
            else {
                /*check all occupied cache lines for a hit*/
                
                temp = c1.LRU_head[index];
                printf("temp = 0x%x\n", temp);

                for (i = 0; i < c1.set_contents[index]; i++){

                    /*cache hit*/
                    if (temp->tag == addr_tag) {
                        /*printf("cache hit\n");*/
                        hit_flag = TRUE;

                        /*insert cache line at head*/
                        if (c1.set_contents[index] > 1){
                            delete(&c1.LRU_head[index], &c1.LRU_tail[index], temp);
                            insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp);
                        }
                        break;
                    }

                    /*we've reached the tail*/
                    if (temp->LRU_next == NULL) {
                        break;
                    }

                    temp = temp->LRU_next;
                }

                /*cache miss*/
                if (hit_flag == FALSE){
                    printf("Cache miss\n");

                    printf("set contents (%d) assoc (%d)\n", c1.set_contents[index], c1.associativity);

                    /*Insert cache Line if one is free*/
                    if (c1.set_contents[index] < c1.associativity){

                        temp = (Pcache_line *)malloc(sizeof(cache_line));
                        temp->tag = addr_tag;
                        temp->dirty  = FALSE;

                        printf("HERE\n");

                        printf("head index tag address (0x%x)\n", &c1.LRU_head[index]->tag);
                        printf("tail index tag address (0x%x)\n", &c1.LRU_tail[index]->tag);
                        printf("temp index tag address (0x%x)\n", &temp->tag);
                        
                        insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp);

                        printf("HERE\n");
                        
                        cache_stat_data.misses += 1;
                        cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                        c1.set_contents[index] += 1;

                    }
                    /*Else use LRU replacement policy*/
                    else if (c1.set_contents[index] == c1.associativity){
                        printf("set full\n");

                        delete(&c1.LRU_head[index], &c1.LRU_tail[index], temp);

                        temp = (Pcache_line *)malloc(sizeof(cache_line));
                        temp->tag = addr_tag;
                        temp->dirty  = FALSE;
                        
                        insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp);
                        
                        cache_stat_data.misses += 1;
                        cache_stat_data.replacements += 1;
                        cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                    }  
                }
            }
        }

        /*Split cache*/
        else {
            printf("c2.LRU_head[index] 0x%x\n", c2.LRU_head[index]);
        
            /*cache miss on NULL*/
            if (c2.LRU_head[index] == NULL) {
                printf("Cache miss on NULL\n");

                temp = (Pcache_line *)malloc(sizeof(cache_line));
                temp->tag = addr_tag;
                temp->dirty = FALSE;
                temp->LRU_prev = NULL;
                temp->LRU_next = NULL;

                c2.LRU_head[index] = temp;
                c2.LRU_tail[index] = temp;

                cache_stat_data.misses += 1;
                cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                c2.set_contents[index] += 1;
            }
            
            else{
                /*check all occupied cache lines for a hit*/
                
                temp = c2.LRU_head[index];

                for (i = 0; i < c2.set_contents[index]; i++){

                    /*cache hit*/
                    if (temp->tag == addr_tag) {
                        /*printf("cache hit\n");*/
                        hit_flag = TRUE;

                        /*insert cache line at head*/
                        if (c2.set_contents[index] > 1){
                            delete(&c2.LRU_head[index], &c2.LRU_tail[index], temp);
                            insert(&c2.LRU_head[index], &c2.LRU_tail[index], temp);
                        }
                        break;
                    }

                    /*we've reached the tail*/
                    if (temp->LRU_next == NULL) {
                        break;
                    }

                    temp = temp->LRU_next;
                }

                /*cache miss*/
                if (hit_flag == FALSE){
                    printf("Cache miss\n");

                    /*Insert cache Line if one is free*/
                    if (c2.set_contents[index] < c2.associativity){

                        temp = (Pcache_line *)malloc(sizeof(cache_line));
                        temp->tag = addr_tag;
                        temp->dirty  = FALSE;
                        
                        insert(&c2.LRU_head[index], &c2.LRU_tail[index], temp);
                        
                        cache_stat_data.misses += 1;
                        cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                        c2.set_contents[index] += 1;

                    }
                    /*Else use LRU replacement policy*/
                    else if (c2.set_contents[index] == c2.associativity){
                        /*printf("set full\n");*/

                        delete(&c2.LRU_head[index], &c2.LRU_tail[index], temp);

                        temp = (Pcache_line *)malloc(sizeof(cache_line));
                        temp->tag = addr_tag;
                        temp->dirty  = FALSE;
                        
                        insert(&c2.LRU_head[index], &c2.LRU_tail[index], temp);
                        
                        cache_stat_data.misses += 1;
                        cache_stat_data.replacements += 1;
                        cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                    }  
                }
            }
        }
    }

    /*data store*/
    if (access_type == 1) {
        
        hit_flag = FALSE;

        printf("Data store\n");
        cache_stat_data.accesses += 1;
        
        /*Unified cache*/
        if (cache_split == FALSE) {
            /*cache miss on NULL*/
            if (c1.LRU_head[index] == NULL){
                
                temp = (Pcache_line *)malloc(sizeof(cache_line));
                temp->tag = addr_tag;
                temp->dirty = FALSE;
                temp->LRU_prev = NULL;
                temp->LRU_next = NULL;

                c1.LRU_head[index] = temp;
                c1.LRU_tail[index] = temp;

                cache_stat_data.misses += 1;
                cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                c1.set_contents[index] += 1;
            }
            
            else{
                /*check all occupied cache lines for a hit*/
                
                temp = c1.LRU_head[index];

                for (i = 0; i < c1.set_contents[index]; i++){

                    /*cache hit*/
                    if (temp->tag == addr_tag) {
                        /*printf("cache hit\n");*/
                        hit_flag = TRUE;

                        /*insert cache line at head*/
                        if (c1.set_contents[index] > 1){
                            delete(&c1.LRU_head[index], &c1.LRU_tail[index], temp);
                            insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp);
                        }
                        break;
                    }

                    /*we've reached the tail*/
                    if (temp->LRU_next == NULL) {
                        break;
                    }

                    temp = temp->LRU_next;
                }

                /*cache miss*/
                if (hit_flag == FALSE){
                    /*printf("Cache miss\n");*/

                    /*Insert cache Line if one is free*/
                    if (c1.set_contents[index] < c1.associativity){

                        temp = (Pcache_line *)malloc(sizeof(cache_line));
                        temp->tag = addr_tag;
                        temp->dirty  = FALSE;
                        
                        insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp);
                        
                        cache_stat_data.misses += 1;
                        cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                        c1.set_contents[index] += 1;

                    }
                    /*Else use LRU replacement policy*/
                    else if (c1.set_contents[index] == c1.associativity){
                        /*printf("set full\n");*/

                        delete(&c1.LRU_head[index], &c1.LRU_tail[index], temp);

                        temp = (Pcache_line *)malloc(sizeof(cache_line));
                        temp->tag = addr_tag;
                        temp->dirty  = FALSE;
                        
                        insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp);
                        
                        cache_stat_data.misses += 1;
                        cache_stat_data.replacements += 1;
                        cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                    }  
                }
            }
        }

        /*Split cache*/
        else {
            /*cache miss on NULL*/
            if (c2.LRU_head[index] == NULL){
                
                temp = (Pcache_line *)malloc(sizeof(cache_line));
                temp->tag = addr_tag;
                temp->dirty = FALSE;
                temp->LRU_prev = NULL;
                temp->LRU_next = NULL;

                c2.LRU_head[index] = temp;
                c2.LRU_tail[index] = temp;

                cache_stat_data.misses += 1;
                cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                c2.set_contents[index] += 1;
            }
            
            else{
                /*check all occupied cache lines for a hit*/
                
                temp = c2.LRU_head[index];

                for (i = 0; i < c2.set_contents[index]; i++){

                    /*cache hit*/
                    if (temp->tag == addr_tag) {
                        /*printf("cache hit\n");*/
                        hit_flag = TRUE;

                        /*insert cache line at head*/
                        if (c2.set_contents[index] > 1){
                            delete(&c2.LRU_head[index], &c2.LRU_tail[index], temp);
                            insert(&c2.LRU_head[index], &c2.LRU_tail[index], temp);
                        }
                        break;
                    }

                    /*we've reached the tail*/
                    if (temp->LRU_next == NULL) {
                        break;
                    }

                    temp = temp->LRU_next;
                }

                /*cache miss*/
                if (hit_flag == FALSE){
                    /*printf("Cache miss\n");*/

                    /*Insert cache Line if one is free*/
                    if (c2.set_contents[index] < c2.associativity){

                        temp = (Pcache_line *)malloc(sizeof(cache_line));
                        temp->tag = addr_tag;
                        temp->dirty  = FALSE;
                        
                        insert(&c2.LRU_head[index], &c2.LRU_tail[index], temp);
                        
                        cache_stat_data.misses += 1;
                        cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                        c2.set_contents[index] += 1;

                    }
                    /*Else use LRU replacement policy*/
                    else if (c2.set_contents[index] == c2.associativity){
                        /*printf("set full\n");*/

                        delete(&c2.LRU_head[index], &c2.LRU_tail[index], temp);

                        temp = (Pcache_line *)malloc(sizeof(cache_line));
                        temp->tag = addr_tag;
                        temp->dirty  = FALSE;
                        
                        insert(&c2.LRU_head[index], &c2.LRU_tail[index], temp);
                        
                        cache_stat_data.misses += 1;
                        cache_stat_data.replacements += 1;
                        cache_stat_data.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                    }  
                }
            }
        }
    }

    /*instruction load*/
    if (access_type == 2) {

        hit_flag = FALSE;
        
        printf("Instruction Load\n");
        cache_stat_inst.accesses += 1;
        
        /*printf("c1.LRU_head[index] 0x%x\n", c1.LRU_head[index]);*/
        
        /*cache miss on NULL*/
        if (c1.LRU_head[index] == NULL) {
            /*printf("Cache miss on NULL\n");*/

            temp = (Pcache_line *)malloc(sizeof(cache_line));
            temp->tag = addr_tag;
            temp->dirty = FALSE;
            temp->LRU_prev = NULL;
            temp->LRU_next = NULL;

            c1.LRU_head[index] = temp;
            c1.LRU_tail[index] = temp;

            cache_stat_inst.misses += 1;
            cache_stat_inst.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
            c1.set_contents[index] += 1;
        }
        
        else{
            /*check all occupied cache lines for a hit*/
            /*printf("Checking occupied cache lines\n");*/
            
            temp = c1.LRU_head[index];

            for (i = 0; i < c1.set_contents[index]; i++){

                /*cache hit*/
                if (temp->tag == addr_tag) {
                    /*printf("cache hit\n");*/
                    hit_flag = TRUE;

                    /*printf("head index tag address (0x%x)\n", &c1.LRU_head[index]->tag);
                    printf("tail index tag address (0x%x)\n", &c1.LRU_tail[index]->tag);
                    printf("temp index tag address (0x%x)\n", &temp->tag);*/

                    /*insert cache line at head*/
                    if (c1.set_contents[index] > 1){
                        delete(&c1.LRU_head[index], &c1.LRU_tail[index], temp);
                        insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp);
                    }
                    break;
                }

                /*we've reached the tail*/
                if (temp->LRU_next == NULL) {
                    break;
                }

                temp = temp->LRU_next;
            }

            /*cache miss*/
            if (hit_flag == FALSE){
                /*printf("Cache miss\n");*/

                /*Insert cache Line if one is free*/
                if (c1.set_contents[index] < c1.associativity){
                    /*
                    printf("Free cache line\n");
                    printf("Set contents (%d) Associativity (%d)\n", c1.set_contents[index], c1.associativity);
                    */

                    temp = (Pcache_line *)malloc(sizeof(cache_line));
                    temp->tag = addr_tag;
                    temp->dirty  = FALSE;
                    
                    insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp);
                    
                    cache_stat_inst.misses += 1;
                    cache_stat_inst.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                    c1.set_contents[index] += 1;

                }
                /*Else use LRU replacement policy*/
                else if (c1.set_contents[index] == c1.associativity){
                    /*printf("set full\n");*/

                    /*
                    printf("head index address (0x%x)\n", &c1.LRU_head[index]->tag);
                    printf("tail index address (0x%x)\n", &c1.LRU_tail[index]->tag);
                    */

                    delete(&c1.LRU_head[index], &c1.LRU_tail[index], temp);

                    temp = (Pcache_line *)malloc(sizeof(cache_line));
                    temp->tag = addr_tag;
                    temp->dirty  = FALSE;
                    
                    insert(&c1.LRU_head[index], &c1.LRU_tail[index], temp);
                    
                    cache_stat_inst.misses += 1;
                    cache_stat_inst.replacements += 1;
                    cache_stat_inst.demand_fetches += 1 * cache_block_size / (WORD_SIZE);
                }  
            }
        }
    }

    return;
}
/************************************************************/

/************************************************************/
void flush()
{

  /* flush the cache */

    /*TODO check dirty bit and write to memory for CB*/

    int i, j;
    Pcache_line temp = (Pcache_line)malloc(sizeof(cache_line));

    for (i = 0; i < c1.n_sets; i++){
        temp = c1.LRU_head[i];
        for (j = 0; j < c1.set_contents[i]; j++)
            if (temp->dirty == TRUE){
                cache_stat_inst.copies_back += 1;
            }
            temp = temp->LRU_next;

    }

    if (cache_split == TRUE){
        for (i = 0; i < c1.n_sets; i++){
            temp = c2.LRU_head[i];
            for (j = 0; j < c2.set_contents[i]; j++)
                if (temp->dirty == TRUE){
                    cache_stat_data.copies_back += 1;
                }   
            temp = temp->LRU_next;

        }
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