/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    // for 32 x 32
    if (M == 32 && N == 32)
    {
        int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
        for (int ii = 0; ii < M; ii += 8)
        {
            for (int jj = 0; jj < N; jj += 8)
            {
                for (int i = 0; i < 8; i++)
                {
                    temp1 = A[ii + i][jj];
                    temp2 = A[ii + i][jj + 1];
                    temp3 = A[ii + i][jj + 2];
                    temp4 = A[ii + i][jj + 3];
                    temp5 = A[ii + i][jj + 4];
                    temp6 = A[ii + i][jj + 5];
                    temp7 = A[ii + i][jj + 6];
                    temp8 = A[ii + i][jj + 7];
                    B[jj][ii + i] = temp1;
                    B[jj + 1][ii + i] = temp2;
                    B[jj + 2][ii + i] = temp3;
                    B[jj + 3][ii + i] = temp4;
                    B[jj + 4][ii + i] = temp5;
                    B[jj + 5][ii + i] = temp6;
                    B[jj + 6][ii + i] = temp7;
                    B[jj + 7][ii + i] = temp8;
                }
            }
        }
    }
    else if (M == 64 && N == 64)
    {
        int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
        for (int ii = 0; ii < M; ii += 8)
        {
            for (int jj = 0; jj < N; jj += 8)
            {
                // A[0] -> cache -> reg
                // B[0] -> kick A[0] off cache
                // B[1] -> kick A[1] and so on
                // store top right of A to B
                for (int i = 0; i < 4; i++)
                {
                    temp1 = A[ii + i][jj];
                    temp2 = A[ii + i][jj + 1];
                    temp3 = A[ii + i][jj + 2];
                    temp4 = A[ii + i][jj + 3];
                    temp5 = A[ii + i][jj + 4];
                    temp6 = A[ii + i][jj + 5];
                    temp7 = A[ii + i][jj + 6];
                    temp8 = A[ii + i][jj + 7];
                    B[jj][ii + i] = temp1;
                    B[jj + 1][ii + i] = temp2;
                    B[jj + 2][ii + i] = temp3;
                    B[jj + 3][ii + i] = temp4;
                    B[jj][ii + i + 4] = temp5;
                    B[jj + 1][ii + i + 4] = temp6;
                    B[jj + 2][ii + i + 4] = temp7;
                    B[jj + 3][ii + i + 4] = temp8;
                }
                // 2. A[TR] <-> B[BL], store A[BR]
                // Now, B[0] - B[3] are in cache
                // Read A[BL] -> reg1 and A[BR] -> reg2
                // Swap B[TR] and reg1, reg2 -> B[BR]
                for (int j = 0; j < 4; j++)
                {
                    temp1 = A[ii + 4][jj + j];
                    temp2 = A[ii + 5][jj + j];
                    temp3 = A[ii + 6][jj + j];
                    temp4 = A[ii + 7][jj + j];
                    temp5 = B[jj + j][ii + 4];
                    temp6 = B[jj + j][ii + 5];
                    temp7 = B[jj + j][ii + 6];
                    temp8 = B[jj + j][ii + 7];
                    B[jj + j][ii + 4] = temp1;
                    B[jj + j][ii + 5] = temp2;
                    B[jj + j][ii + 6] = temp3;
                    B[jj + j][ii + 7] = temp4;
                    B[jj + j + 4][ii] = temp5;
                    B[jj + j + 4][ii + 1] = temp6;
                    B[jj + j + 4][ii + 2] = temp7;
                    B[jj + j + 4][ii + 3] = temp8;
                }
                // A[BR] -> B[BR]
                for (int j = 0; j < 4; j++)
                {
                    temp1 = A[ii + 4][jj + 4 + j];
                    temp2 = A[ii + 5][jj + 4 + j];
                    temp3 = A[ii + 6][jj + 4 + j];
                    temp4 = A[ii + 7][jj + 4 + j];
                    B[jj + 4 + j][ii + 4] = temp1;
                    B[jj + 4 + j][ii + 5] = temp2;
                    B[jj + 4 + j][ii + 6] = temp3;
                    B[jj + 4 + j][ii + 7] = temp4;
                }
            }
        }
    }
    else
    {
        for (int ii = 0; ii < N; ii += 16)
        {
            for (int jj = 0; jj < M; jj += 16)
            {
                for (int i = ii; i < ii + 16 && i < N; i++)
                {
                    for (int j = jj; j < jj + 16 && j < M; j++)
                    {
                        B[j][i] = A[i][j];
                    }
                }
            }
        }
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
