/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
       R A N K  A L G O R I T H M  F U N C T I O N  P R O T O T Y P E S 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdint.h>

int32_t			computeRank(int32_t M, int32_t Q, uint8_t **matrix);
void			perform_elementary_row_operations(int32_t flag, int32_t i, int32_t M, int32_t Q, uint8_t **A);
int32_t			find_unit_element_and_swap(int32_t flag, int32_t i, int32_t M, int32_t Q, uint8_t **A);
int32_t			swap_rows(int32_t i, int32_t index, int32_t Q, uint8_t **A);
int32_t			determine_rank(int32_t m, int32_t M, int32_t Q, uint8_t **A);
uint8_t**	    create_matrix(int32_t M, int32_t Q);
void			display_matrix(int32_t M, int32_t Q, uint8_t **m);
void			def_matrix(uint8_t* bitstreamBuffer, int32_t M, int32_t Q, uint8_t **m,int32_t k);
void			delete_matrix(int32_t M, uint8_t **matrix);