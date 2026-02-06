#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct
{
    bool valid;
    long tag;
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
    unsigned address;
    // store size of accessed memory
    int size;

    while (fscanf(pFILE, "%c %x,%d", &identifier, &address, &size) > 0)
    {
    }

    fclose(pFILE);

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