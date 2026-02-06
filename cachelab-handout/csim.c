/*
Takes a valgrind memory traces as input, simulates hit/miss beharior of a cache memory on this trace, and outputs the total number of hits, misses, and evictions
*/

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct
{
    bool valid;
    long tag;
    long counter;
} cache_line;

/*
Use this function to init cache
return NULL if allocation fails.
*/
cache_line **cache_init(int S, int E);
/*
Use this function to delet cache.
*/
void cache_del(int S, cache_line **cache);

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

    // For Debug
    printf("[DEBUG]:\ns: %d\nE: %d\nb: %d\ntracefile: %s\n", s, E, b, t);

    // Allocate cache memory
    int S = 1 << s; // S = 2^s
    cache_line **cache = cache_init(S, E);

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
    long global_counter = 0;
    int num_hits = 0, num_misses = 0, num_evictions = 0;

    while (fscanf(pFILE, " %c %lx,%d", &identifier, &address, &size) > 0) // Add a space before %c to consume the newline character
    {
        printf("[DEBUG] global_counter: %ld\n", global_counter);

        // get tag, set and block_offset of this address
        long tag = address >> (s + b);
        long set_idx = (address >> b) & ((1 << s) - 1);
        printf("[DEBUG] address: %lx\n", address);
        printf("[DEBUG] tag: %ld\n", tag);
        printf("[DEBUG] set_idx: %ld\n", set_idx);

        if (identifier == 'L')
        {
        }
        else if (identifier == 'S')
        {
        }
        else if (identifier == 'M')
        {
        }
        else
        {
            fprintf(stderr, "Error: invalid identifier: %c\n", identifier);
        }

        global_counter++;
    }

    // close file and free memory
    fclose(pFILE);
    cache_del(S, cache);

    printSummary(num_hits, num_misses, num_evictions);

    return 0;
}

cache_line **cache_init(int S, int E)
{
    cache_line **cache = malloc(S * sizeof(cache_line *));
    if (cache == NULL)
        return NULL;
    for (int s = 0; s < S; s++)
    {
        cache[s] = malloc(E * sizeof(cache_line));
    }
    if (cache == NULL)
        return NULL;

    // init contents of cache
    for (int i = 0; i < S; i++)
    {
        for (int j = 0; j < E; j++)
        {
            cache[i][j].valid = false;
            cache[i][j].tag = 0;
            cache[i][j].counter = 0;
        }
    }
    return cache;
}

void cache_del(int S, cache_line **cache)
{
    for (int s = 0; s < S; s++)
    {
        free(cache[s]);
    }
    free(cache);
}