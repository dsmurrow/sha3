#include "sponge.h"

#include <stdlib.h>

uint8_t *sponge(const spng_f_t *f, uint8_t *(*pad)(uint32_t, uint32_t, uint32_t*), uint32_t r, const uint8_t *N, uint32_t n_bitlen, uint32_t d)
{
	uint8_t *padding, **P_i, *S, *Z;
	uint32_t p_bitlen, c, n, i;

	if(f == NULL || pad == NULL || N == NULL || r == 0 || n_bitlen == 0 || r >= f->b) return NULL;

	padding = pad(r, n_bitlen, &p_bitlen);

	p_bitlen += n_bitlen;

	n = p_bitlen / r; /* n = len(P) / r */

	c = f->b - r; /* c = b - r */

	P_i = malloc(n * sizeof(uint8_t*));
	if(P_i == NULL)
	{
		free(padding);
		return NULL;
	}

	/* Instead of assigning P and then splitting it into the values for P_i,
	 * I'm only allocating the memory for P_i and filling it in there. */
	for(i = 0; i < n; i++)
	{
		P_i[i] = calloc((r / 8) + (r % 8 > 0), sizeof(uint8_t));
		if(P_i[i] == NULL)
		{
			free(padding);
			for(c = 0; c < i; c++) free(P_i[c]);
			free(P_i);

			return NULL;
		}
	}


	for(i = 0; i < n_bitlen; i++)
	{
		P_i[i / r][(i % r) / 8] |= ((N[i / 8] >> (7 - (i % 8))) & 1) << (7 - ((i % r) % 8));
	}

	for(; i < p_bitlen; i++)
	{
		P_i[i / r][(i % r) / 8] |= ((padding[(i - n_bitlen) / 8] >> (7 - ((i - n_bitlen) % 8))) & 1) << (7 - ((i % r) % 8));
	}

	free(padding);


	S = calloc((f->b / 8) + (f->b % 8 > 0), sizeof(uint8_t)); /* S= 0^b */
	if(S == NULL)
	{
		for(i = 0; i < n; i++) free(P_i[i]);
		free(P_i);

		return NULL;
	}



	/* For i from 0 to n-1, let S=f(S ^ (P_i || 0^c)) */
	for(i = 0; i < n; i++)
	{
		/* c used as iterator now because it has served its original purpose */
		for(c = 0; c < (r / 8) + (r % 8 > 0); c++)
			S[c] ^= P_i[i][c];

		/* padding variable used to hold new S value so old S can be freed */
		padding = f->f(f->b, S);
		free(S);
		free(P_i[i]); /* P_i[i] no longer needed */
		S = padding;
	}
	free(P_i);

	/* Let Z be the empty string (implemented as Z=0^d) */	
	Z = calloc((d / 8) + (d % 8 > 0), sizeof(uint8_t));
	if(Z == NULL)
	{
		free(S);
		return NULL;
	}

	n_bitlen = 0; /* n_bitlen now serves as z_bitlen */
	while(1)
	{
		for(i = 0; i < r && n_bitlen < d; i++, n_bitlen++)
		{
			Z[n_bitlen / 8] |= ((S[i / 8] >> (7 - (i % 8))) & 1) << (7 - (n_bitlen % 8));
		}

		if(n_bitlen == d) break;

		padding = f->f(f->b, S); /* padding holds new value of S */
		free(S);
		S = padding;
	}

	free(S);

	return Z;
}

