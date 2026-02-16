/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define WSIZE 4
#define CHUNKSIZE 4096

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// read or write 4 byte data in address p
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
// get size or malloc information from header or footer
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
// PACK: pack size and allocation bit. alloc: 0 -> unallocated, 1 -> allocated
#define PACK(size, alloc) ((size) | (alloc))
// HDPR: get header pointer
#define HDPR(p) ((char *)(p) - WSIZE)
// FTPR: get footer pointer
#define FTPR(p) (HDPR(p) + GET_SIZE(HDPR(p)) - WSIZE)
// NEXTBLKP: jump to the next block pointer
#define NEXTBLKP(p) ((char *)(p) + GET_SIZE(HDPR(p)))
// PREVBLKP: jump to the previous block pointer
#define PREVBLKP(p) ((char *)(p) - GET_SIZE(HDPR(p) - WSIZE))
#define NEXTFREE(p) ((char *)(p) + WSIZE)
#define PREVFREE(p) ((char *)(p))
// For pointer read
#define GET_PTR(p) (*(char **)(p))
#define PUT_PTR(p, ptr) (*(char **)(p) = (ptr))

#define SEGNUM 20

// heap list pointer
static char *heap_listp = 0;

// segreagted list
// seg_list[0]: < 2^5 = 32
// seg_list[1]: < 2^6 = 64
// seg_list[2]: < 2^7 = 128
// seg_list[3]: >= 128
static char *seg_list[SEGNUM];

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t size);
static void *coalesce(void *bp);
static void place(void *bp, size_t size);
static int get_seg_index(size_t size);

/*
 * remove_from_freelist - Removes a block from the free list
 * bp: Address of the block to be removed
 */
static inline void remove_from_freelist(void *bp)
{
    size_t size = GET_SIZE(HDPR(bp));
    int index = get_seg_index(size);

    void *prev_free = GET_PTR(PREVFREE(bp));
    void *next_free = GET_PTR(NEXTFREE(bp));

    if (prev_free != NULL)
    {
        PUT_PTR(NEXTFREE(prev_free), next_free);
    }
    else
    {
        seg_list[index] = next_free;
    }

    if (next_free != NULL)
    {
        PUT_PTR(PREVFREE(next_free), prev_free);
    }
}

static inline void insert_free_block(void *bp)
{
    size_t size = GET_SIZE(HDPR(bp));
    int index = get_seg_index(size);
    void *search_ptr = seg_list[index];

    PUT_PTR(NEXTFREE(bp), search_ptr);
    PUT_PTR(PREVFREE(bp), NULL);
    if (search_ptr != NULL)
    {
        PUT(PREVFREE(search_ptr), bp);
    }
    seg_list[index] = bp;
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    for (int i = 0; i < SEGNUM; i++)
    {
        seg_list[i] = NULL;
    }
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
    {
        // If allocation fails
        return -1;
    }

    // padding information
    PUT(heap_listp, 0);                                // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(ALIGNMENT, 1)); // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(ALIGNMENT, 1)); // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));         // Epilogue header

    // move heap list pointer
    heap_listp += (2 * WSIZE);

    // extend heap
    if (extend_heap(CHUNKSIZE) == NULL)
    {
        return -1;
    }
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; // Adjusted block size
    char *ptr;

    if (size == 0)
    {
        return NULL;
    }
    else
    {
        // Adjust block size to include overhead and alignment
        asize = ALIGN(size + 2 * WSIZE);
        // Minimum block size is 6 * WSIZE (header + footer + min payload)
        if (asize < 4 * WSIZE)
            asize = 4 * WSIZE;
    }

    int index = get_seg_index(asize);

    // Starting from most fit bucket, if not found, go to bigger bucket
    for (int i = index; i < SEGNUM; i++)
    {
        ptr = seg_list[i];
        void *best_fit = NULL;
        size_t min_diff = (size_t)-1;
        while (ptr != NULL)
        {
            size_t curr_size = GET_SIZE(HDPR(ptr));
            if (curr_size >= asize)
            {
                size_t diff = curr_size - asize;
                if (diff < min_diff)
                {
                    min_diff = diff;
                    best_fit = ptr;
                    if (diff == 0)
                        break; // 完美匹配，直接跳出
                }
            }
            ptr = GET_PTR(NEXTFREE(ptr));
        }
        if (best_fit != NULL)
        {
            place(best_fit, asize);
            return best_fit;
        }
    }

    // Else, extend_heap
    if ((ptr = extend_heap(asize)) == NULL)
    {
        return NULL;
    }

    place(ptr, asize);
    return ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    // IMPORTANT: mark current block as freed!
    size_t size = GET_SIZE(HDPR(ptr));

    PUT(HDPR(ptr), PACK(size, 0));
    PUT(FTPR(ptr), PACK(size, 0));
    PUT(PREVFREE(ptr), NULL);
    PUT(NEXTFREE(ptr), NULL);
    // merge it when freeing
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    // 1. If original pointer is NULL
    if (ptr == NULL)
    {
        return mm_malloc(size);
    }
    // 2. If new size is 0
    if (size == 0)
    {
        return NULL;
    }

    size_t oldsize = GET_SIZE(HDPR(ptr));
    size_t newsize = ALIGN(size + 2 * WSIZE);
    void *next_blk = NEXTBLKP(ptr);
    size_t next_size = GET_SIZE(HDPR(next_blk));
    int next_alloc = GET_ALLOC(HDPR(next_blk));
    void *prev_blk = PREVBLKP(ptr);
    size_t prev_size = GET_SIZE(HDPR(prev_blk));
    int prev_alloc = GET_ALLOC(HDPR(prev_blk));

    // 1. If newsize < oldsize, place directly;
    if (newsize <= oldsize)
    {
        return ptr;
    }
    // 2. If newsize > oldsize, and there is space available nearby
    else if (!GET_ALLOC(HDPR(next_blk)) && next_size >= newsize - oldsize)
    {
        remove_from_freelist(next_blk);

        PUT(HDPR(ptr), PACK(oldsize + next_size, 1));
        PUT(FTPR(ptr), PACK(oldsize + next_size, 1));

        return ptr;
    }
    // 3. there is space available ahead
    // Case 2: 向前合并 (新增)
    // 注意：如果前一块是空闲的，且 前+中 足够大
    else if (!prev_alloc && (oldsize + prev_size >= newsize))
    {
        remove_from_freelist(prev_blk);
        void *newptr = prev_blk;
        // 移动数据：因为地址重叠，必须用 memmove 而不是 memcpy
        memmove(newptr, ptr, oldsize - 2 * WSIZE);
        PUT(HDPR(newptr), PACK(oldsize + prev_size, 1));
        PUT(FTPR(newptr), PACK(oldsize + prev_size, 1));
        // TODO: 同样，如果剩余空间很大，记得分割 (Split)
        return newptr;
    }

    // 4. Case 3: Extend to Both Prev and Next (New!)
    else if (!prev_alloc && !next_alloc && (prev_size + oldsize + next_size >= newsize))
    {
        remove_from_freelist(prev_blk);
        remove_from_freelist(next_blk);

        void *newptr = prev_blk;
        memmove(newptr, ptr, oldsize - 2 * WSIZE);

        size_t total_size = prev_size + oldsize + next_size;

        // 分割逻辑
        if (total_size - newsize >= 4 * WSIZE)
        {
            PUT(HDPR(newptr), PACK(newsize, 1));
            PUT(FTPR(newptr), PACK(newsize, 1));

            void *new_free_blk = NEXTBLKP(newptr);
            PUT(HDPR(new_free_blk), PACK(total_size - newsize, 0));
            PUT(FTPR(new_free_blk), PACK(total_size - newsize, 0));
            insert_free_block(new_free_blk);
        }
        else
        {
            PUT(HDPR(newptr), PACK(total_size, 1));
            PUT(FTPR(newptr), PACK(total_size, 1));
        }
        return newptr;
    }

    // 5. malloc + copy + free
    else
    {
        void *newptr = mm_malloc(size);
        if (newptr == NULL)
        {
            return NULL;
        }
        memcpy(newptr, ptr, oldsize - 2 * WSIZE);
        mm_free(ptr);
        return newptr;
    }
}

void *extend_heap(size_t size)
{
    char *bp;
    size = ALIGN(size);

    if ((long)(bp = mem_sbrk(size)) == -1)
    {
        return NULL;
    }

    // Overwrite old epilogue header with new empty block's
    PUT(HDPR(bp), PACK(size, 0));
    // Write new footer of new block
    PUT(FTPR(bp), PACK(size, 0));
    // Writer new epilogue
    PUT(HDPR(NEXTBLKP(bp)), PACK(0, 1));

    // try merge previous block
    return coalesce(bp);
}

void *coalesce(void *bp)
{
    // Get status of adjacent blocks
    // Read prev block's footer directly to avoid accessing invalid memory
    size_t prev_alloc = GET_ALLOC((char *)(bp)-2 * WSIZE);
    size_t next_alloc = GET_ALLOC(HDPR(NEXTBLKP(bp)));
    size_t size = GET_SIZE(HDPR(bp));

    // case 1: both next and prev have been allocated
    if (prev_alloc && next_alloc)
    {
        insert_free_block(bp);
        return bp;
    }

    // case 2: prev allocated, next free -> merge next
    else if (prev_alloc && !next_alloc)
    {
        remove_from_freelist(NEXTBLKP(bp));
        size += GET_SIZE(HDPR(NEXTBLKP(bp)));
        PUT(HDPR(bp), PACK(size, 0));
        PUT(FTPR(bp), PACK(size, 0));
    }

    // case 3: prev free, next allocated -> merge prev
    else if (!prev_alloc && next_alloc)
    {
        remove_from_freelist(PREVBLKP(bp));
        size += GET_SIZE(HDPR(PREVBLKP(bp)));
        bp = PREVBLKP(bp);
        PUT(HDPR(bp), PACK(size, 0));
        PUT(FTPR(bp), PACK(size, 0));
    }

    // case 4: both are free
    else
    {
        remove_from_freelist(NEXTBLKP(bp));
        remove_from_freelist(PREVBLKP(bp));
        size += GET_SIZE(HDPR(PREVBLKP(bp)));
        size += GET_SIZE(HDPR(NEXTBLKP(bp)));
        bp = PREVBLKP(bp);
        PUT(HDPR(bp), PACK(size, 0));
        PUT(FTPR(bp), PACK(size, 0));
    }

    insert_free_block(bp);
    return bp;
}

void place(void *bp, size_t size)
{
    size_t current_size = GET_SIZE(HDPR(bp));
    size_t min_block_size = 4 * WSIZE;

    // Remove from free list first
    remove_from_freelist(bp);

    if (current_size - size < min_block_size)
    {
        // Return all this block
        PUT(HDPR(bp), PACK(current_size, 1));
        PUT(FTPR(bp), PACK(current_size, 1));
    }
    else
    {
        // Split: allocate first part, keep second part free
        PUT(HDPR(bp), PACK(size, 1));
        PUT(FTPR(bp), PACK(size, 1));

        void *next_bp = NEXTBLKP(bp);
        PUT(HDPR(next_bp), PACK(current_size - size, 0));
        PUT(FTPR(next_bp), PACK(current_size - size, 0));

        // Insert the remaining free block back to free list
        insert_free_block(next_bp);
    }
}

int get_seg_index(size_t size)
{
    // -----------------------------------------------------------
    // 策略 1: 小块线性分桶 (Linear Bucketing for Small Blocks)
    // 目标：让 16, 24, 32... 这些常见小块都有独立的桶，互不干扰
    // -----------------------------------------------------------

    // 这里的 64 是个阈值，你可以根据 trace 调整 (比如 96 或 128)
    if (size <= 64)
    {
        // 原理：size 是 8 的倍数。
        // size = 16 -> (16 >> 3) = 2 -> 2 - 2 = 0 (Index 0)
        // size = 24 -> (24 >> 3) = 3 -> 3 - 2 = 1 (Index 1)
        // size = 32 -> (32 >> 3) = 4 -> 4 - 2 = 2 (Index 2)
        // ...
        // size = 64 -> (64 >> 3) = 8 -> 8 - 2 = 6 (Index 6)
        return (size >> 3) - 2;
    }
    // -----------------------------------------------------------
    // 策略 2: 大块对数分桶 (Logarithmic Bucketing for Large Blocks)
    // 目标：大块不需要分那么细，按 2 的幂次分桶即可
    // -----------------------------------------------------------

    // 计算 log2(size)
    // size > 64，所以 31 - clz(size) 至少是 7 (因为 2^6 = 64, 2^7 = 128)
    int index = 31 - __builtin_clz((unsigned int)size);

    // 我们的小块桶用了 0 到 6，所以大块从 7 开始接上
    // index = 7 (size 65-127) -> 对应 bucket 7
    // index = 8 (size 128-255) -> 对应 bucket 8
    // ...

    if (index >= SEGNUM)
        index = SEGNUM - 1;
    return index;
}