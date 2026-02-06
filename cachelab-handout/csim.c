/**
 * cachelab.c - A cache simulator that takes memory traces as input,
 * simulates the hit/miss behavior of a cache memory, and outputs the total
 * number of hits, misses and evictions.
 */

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/**
 * @brief Represents a single line within a cache set.
 */
typedef struct
{
    bool valid;   /**< Valid bit: true if the line contains cached data */
    long tag;     /**< Tag bits to identify the memory block */
    long counter; /**< LRU counter: stores the timestamp of the last access */
} cache_line;

/**
 * @brief Represents the entire cache structure.
 */
typedef struct
{
    cache_line **sets; /**< 2D array of cache lines [S][E] */
    int S;             /**< Number of sets (S = 2^s) */
    int E;             /**< Associativity (number of lines per set) */
    int B;             /**< Block size in bytes (B = 2^b) */
    int num_hits;      /**< Total number of cache hits */
    int num_misses;    /**< Total number of cache misses */
    int num_evictions; /**< Total number of cache evictions */
} Cache;

/**
 * @brief Initializes the cache simulator by allocating memory.
 * @param S Number of sets.
 * @param E Number of lines per set.
 * @param b Number of block bits.
 * @return Pointer to the initialized Cache, or NULL if allocation fails.
 */
Cache *cache_init(int S, int E, int b);

/**
 * @brief Frees all memory associated with the cache.
 * @param cache Pointer to the cache to be deleted.
 */
void cache_del(Cache *cache);

/**
 * @brief Core logic for cache access (Hit/Miss/Eviction).
 * Since this lab doesn't require actual data movement, this function
 * only updates the metadata and counters.
 */
void access_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache);

/**
 * @brief Wrapper for Data Load operation.
 */
void load_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache);

/**
 * @brief Wrapper for Data Store operation.
 */
void store_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache);

/**
 * @brief Wrapper for Data Modify operation (Load + Store).
 * A modify operation always results in at least one access, and the
 * subsequent store is guaranteed to be a hit.
 */
void modify_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache);

int main(int argc, char *argv[])
{
    int opt;
    int s, E, b;
    char *t;

    // Parse command line arguments using getopt
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
            printf("Usage: ./csim -s <s> -E <E> -b <b> -t <tracefile>\n");
            break;
        }
    }

    // Initialize cache structure
    int S = 1 << s;
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
            load_cache(tag, set_idx, block_offset, size, global_counter, cache);
        }
        else if (identifier == 'S')
        {
            store_cache(tag, set_idx, block_offset, size, global_counter, cache);
        }
        else if (identifier == 'M')
        {
            modify_cache(tag, set_idx, block_offset, size, global_counter, cache);
        }
        // Note: 'I' (Instruction load) is ignored per lab instructions

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

    // 1. Search for a Cache Hit
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

    // 2. Cache Miss occurred
    cache->num_misses++;
    int oldest_counter = global_counter;

    // 3. Search for an empty line (Cold Miss)
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

    // 4. Cache Eviction required (Conflict/Capacity Miss)
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

void load_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache)
{
    access_cache(tag, set_idx, block_offset, size, global_counter, cache);
}

void store_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache)
{
    access_cache(tag, set_idx, block_offset, size, global_counter, cache);
}

void modify_cache(unsigned long tag, unsigned long set_idx, unsigned long block_offset, int size, unsigned long global_counter, Cache *cache)
{
    load_cache(tag, set_idx, block_offset, size, global_counter, cache);
    store_cache(tag, set_idx, block_offset, size, global_counter, cache);
}