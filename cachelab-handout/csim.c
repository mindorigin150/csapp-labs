/*
Takes a valgrind memory traces as input, simulates hit/miss beharior of a cache memory on this trace, and outputs the total number of hits, misses, and evictions
*/

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct
{
    bool valid;
    long tag;
    long counter;
} cache_line;

typedef struct
{
    cache_line **sets;
    int S;
    int E;
    int B; // # bytes / block
    int num_hits;
    int num_misses;
    int num_evictions;
} Cache;

/*
Use this function to init cache
return NULL if allocation fails.
*/
Cache *cache_init(int S, int E, int b);
/*
Use this function to delet cache.
*/
void cache_del(Cache *cache);
/**
 * Load from cache.
 * There are 3 possibilities:
 * - 1 hit
 * - 1 miss, 0 eviction
 * - 1 miss, 1 eviction
 */
void load_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache); // TODO
/**
 * Store into cache.
 * There are 3 possibilities:
 * - 1 hit
 * - 1 miss, 0 eviction
 * - 1 miss, 1 eviction
 */
void store_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache); // TODO
/**
 * Modify cache.
 * 1 load + 1 store, but after loadting, the store operation must hit.
 * There are 3 possibilities:
 * - 2 hit
 * - 1 miss, 0 eviction, 1 hit
 * - 1 miss, 1 eviction, 1 hit
 */
void modify_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache); // TODO
/**
 * A simple access function
 */
void access_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache);

int main(int argc, char *argv[])
{
    int opt;
    int s, E, b;
    char *t;

    while (-1 != (opt = getopt(argc, argv, "s:E:b:t:")))
    {
        switch (opt)
        {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            t = optarg;
        default:
            printf("Wrong argument\n");
            break;
        }
    }

    // Allocate cache memory
    int S = 1 << s; // S = 2^s
    Cache *cache = cache_init(S, E, b);

    // Open the file
    FILE *pFILE = fopen(t, "r");
    if (pFILE == NULL)
    {
        fprintf(stderr, "Error: Could not open file: %s\n", t);
        return 1;
    }

    // Reading lines like "M 20,1" or "L 19,3"
    // store type of operation, like 'L', 'S', 'M'
    char identifier;
    // store memory adress
    unsigned long address;
    // store size of accessed memory
    int size;
    // indicate the current time; increment at the end of each loop
    unsigned long global_counter = 0;

    while (fscanf(pFILE, " %c %lx,%d", &identifier, &address, &size) > 0) // Add a space before %c to consume the newline character
    {
        // get tag, set and block_offset of this address
        unsigned long tag = address >> (s + b);
        unsigned long set_idx = (address >> b) & ((1 << s) - 1);
        unsigned long block_offset = address & ((1 << b) - 1);

        if (identifier == 'L')
        {
            access_cache(tag, set_idx, block_offset, size, global_counter, cache);
        }
        else if (identifier == 'S')
        {
            access_cache(tag, set_idx, block_offset, size, global_counter, cache);
        }
        else if (identifier == 'M')
        {
            access_cache(tag, set_idx, block_offset, size, global_counter, cache);
            access_cache(tag, set_idx, block_offset, size, global_counter, cache);
        }
        else
        {
            fprintf(stderr, "Error: invalid identifier: %c\n", identifier);
        }

        global_counter++;
    }

    // close file and free memory
    fclose(pFILE);
    cache_del(cache);

    printSummary(cache->num_hits, cache->num_misses, cache->num_evictions);

    return 0;
}

Cache *cache_init(int S, int E, int b)
{
    // allocate space for cache
    Cache *cache = malloc(sizeof(Cache));
    if (cache == NULL)
        return NULL;

    cache->sets = malloc(S * sizeof(cache_line *));
    if (cache->sets == NULL)
        return NULL;

    for (int s = 0; s < S; s++)
    {
        cache->sets[s] = malloc(E * sizeof(cache_line));
        if (cache->sets[s] == NULL)
            return NULL;
    }

    // init contents of cache
    cache->S = S;
    cache->E = E;
    cache->B = 1 << b;
    cache->num_hits = 0;
    cache->num_misses = 0;
    cache->num_evictions = 0;
    for (int i = 0; i < S; i++)
    {
        for (int j = 0; j < E; j++)
        {
            cache->sets[i][j].valid = false;
            cache->sets[i][j].tag = 0;
            cache->sets[i][j].counter = 0;
        }
    }
    return cache;
}

void cache_del(Cache *cache)
{
    for (int s = 0; s < cache->S; s++)
    {
        free(cache->sets[s]);
    }
    free(cache->sets);
    free(cache);
}

void access_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache)
{
    cache_line *set = cache->sets[set_idx];

    for (int i = 0; i < cache->E; i++)
    {
        // only valid and tag matches: hit
        if (set[i].valid == true && set[i].tag == tag)
        {
            cache->num_hits++;
            // update counter
            set[i].counter = global_counter;
            return;
        }
    }

    // no hit, miss ++
    cache->num_misses++;
    int oldest_counter = global_counter;

    // check if eviction and find the smallest counter (oldest)
    for (int i = 0; i < cache->E; i++)
    {
        if (set[i].valid == false)
        {
            // update valid, tag and counter
            set[i].valid = true;
            set[i].tag = tag;
            set[i].counter = global_counter;
            return;
        }
        else
        {
            if (set[i].counter < oldest_counter)
            {
                oldest_counter = set[i].counter;
            }
        }
    }

    // need eviction
    cache->num_evictions++;
    for (int i = 0; i < cache->E; i++)
    {
        if (set[i].counter == oldest_counter)
        {
            set[i].tag = tag;
            set[i].counter = global_counter;
        }
    }
}