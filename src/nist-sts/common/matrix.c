#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "matrix.h"
#include "steer_utilities.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
R A N K  A L G O R I T H M  R O U T I N E S
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define	MATRIX_FORWARD_ELIMINATION	0
#define	MATRIX_BACKWARD_ELIMINATION	1

int32_t
computeRank (int32_t M, 
             int32_t Q, 
             uint8_t **matrix)
{
	int32_t i = 0;
    int32_t rank = 0;
    int32_t m = MIN(M,Q);
	
	/* FORWARD APPLICATION OF ELEMENTARY ROW OPERATIONS */ 
	for (i = 0; i < m - 1; i++) 
    {
		if (matrix[i][i] == 1) 
			perform_elementary_row_operations(MATRIX_FORWARD_ELIMINATION, i, M, Q, matrix);
		else 
        { 	/* matrix[i][i] = 0 */
			if (find_unit_element_and_swap(MATRIX_FORWARD_ELIMINATION, i, M, Q, matrix) == 1) 
				perform_elementary_row_operations(MATRIX_FORWARD_ELIMINATION, i, M, Q, matrix);
		}
	}

	/* BACKWARD APPLICATION OF ELEMENTARY ROW OPERATIONS */ 
	for (i = m - 1; i > 0; i--) 
    {
		if (matrix[i][i] == 1)
			perform_elementary_row_operations(MATRIX_BACKWARD_ELIMINATION, i, M, Q, matrix);
		else 
        { 	/* matrix[i][i] = 0 */
			if (find_unit_element_and_swap(MATRIX_BACKWARD_ELIMINATION, i, M, Q, matrix) == 1)
				perform_elementary_row_operations(MATRIX_BACKWARD_ELIMINATION, i, M, Q, matrix);
		}
	} 

	rank = determine_rank(m, M, Q, matrix);

	return rank;
}

void
perform_elementary_row_operations (int32_t flag, 
                                   int32_t i, 
                                   int32_t M, 
                                   int32_t Q, 
                                   uint8_t **A)
{
	int32_t j = 0;
    int32_t k = 0;
	
	if (flag == MATRIX_FORWARD_ELIMINATION) 
    {
		for (j = i + 1; j < M; j++)
			if (A[j][i] == 1) 
				for (k = i; k < Q; k++) 
					A[j][k] = (A[j][k] + A[i][k]) % 2;
	}
	else 
    {
		for (j = i - 1; j >= 0;  j--)
			if (A[j][i] == 1)
				for (k = 0; k < Q; k++)
					A[j][k] = (A[j][k] + A[i][k]) % 2;
	}
}

int32_t
find_unit_element_and_swap (int32_t flag, 
                            int32_t i, 
                            int32_t M, 
                            int32_t Q, 
                            uint8_t **A)
{ 
	int32_t index = 0;
    int32_t row_op = 0;
	
	if (flag == MATRIX_FORWARD_ELIMINATION) 
    {
		index = i + 1;
		while ((index < M) && (A[index][i] == 0)) 
			index++;
        if (index < M)
            row_op = swap_rows(i, index, Q, A);
	}
	else 
    {
		index = i - 1;
		while ((index >= 0) && (A[index][i] == 0)) 
			index--;
        if (index >= 0)
            row_op = swap_rows(i, index, Q, A);
	}
	
	return row_op;
}

int32_t
swap_rows (int32_t i, 
           int32_t index, 
           int32_t Q, 
           uint8_t **A)
{
	int32_t p = 0;
	uint8_t	temp = 0;
	
	for (p = 0; p < Q; p++) 
    {
		temp = A[i][p];
		A[i][p] = A[index][p];
		A[index][p] = temp;
	}
	
	return 1;
}

int32_t
determine_rank(int32_t m, int32_t M, int32_t Q, uint8_t **A)
{
	int32_t i = 0;
    int32_t j = 0;
    int32_t rank = 0;
    int32_t allZeroes = 0;
	
	/* DETERMINE RANK, THAT IS, COUNT THE NUMBER OF NONZERO ROWS */
	
	rank = m;
	for (i = 0; i < M; i++) 
    {
		allZeroes = 1; 
		for (j = 0; j < Q; j++)  
        {
			if (A[i][j] == 1) 
            {
				allZeroes = 0;
				break;
			}
		}
		if (allZeroes == 1)
			rank--;
	} 
	
	return rank;
}

uint8_t**
create_matrix(int32_t M, int32_t Q)
{
	int32_t i = 0;
	uint8_t	**matrix = NULL;
    int32_t result = STEER_RESULT_SUCCESS;
	
    matrix = (uint8_t **) calloc(M, sizeof(uint8_t *));
	result = STEER_CHECK_CONDITION((matrix != NULL), errno);
    if (result == STEER_RESULT_SUCCESS)
    {
		for (i = 0; i < M; i++ ) 
        {
            matrix[i] = calloc(Q, sizeof(uint8_t));
			result = STEER_CHECK_CONDITION((matrix[i] != NULL), errno);
            if (result != STEER_RESULT_SUCCESS)
            {
                // Free any previously allocated matrices
                if (i > 0)
                {
                    delete_matrix(i, matrix);
                    matrix = NULL;
                    break;
                }
			}
		}
	}
    return matrix;
}

void
def_matrix (uint8_t* bitstreamBuffer, 
            int32_t M, 
            int32_t Q, 
            uint8_t **m, 
            int32_t k)
{
	int32_t i = 0;
    int32_t j = 0;
	
	for (i = 0; i < M; i++) 
		for (j = 0; j < Q; j++)
			m[i][j] = bitstreamBuffer[k*(M*Q)+j+i*M];
}

void
delete_matrix (int32_t M, 
               uint8_t **matrix)
{
	int32_t i = 0;

    if (matrix != NULL)
    {
        for (i = 0; i < M; i++)
        {
            if (matrix[i] != NULL)
            {
                free(matrix[i]);
                matrix[i] = NULL;
            }
        }

        free(matrix);
        matrix = NULL;
    }
}
