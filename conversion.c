#include "conversion.h"

#include <stdlib.h>

uint8_t *h2b(const uint8_t *H, uint32_t m, uint32_t n)
{
	uint8_t *T, j, bit;
	uint32_t i;

	if(H == NULL) return NULL;

	if(n > 8 * m || !n) n = 8 * m;

	T = calloc((n / 8) + (n % 8 != 0), sizeof(uint8_t));
	if(T == NULL) return NULL;


	for(i = 0; i < m && 8 * i + j < n; i++)
	{
		for(j = 0; j < 8 && 8 * i + j < n; j++)
		{
			bit = (H[i] >> j) & 1;
			T[i] |= bit << (7 - j);
		}

		j = 0;
	}

	return T;
}

uint8_t *b2h(const uint8_t *S, uint32_t n)
{
	uint8_t *H, bit;
	uint32_t m, i;

	if(S == NULL) return NULL;

	m = (n / 8) + (n % 8 != 0);

	H = calloc(m, sizeof(uint8_t));

	for(i = 0; i < n; i++)
	{
		bit = (S[i / 8] >> (7 - (i % 8))) & 1;
		H[i / 8] |= bit << (i % 8);
	}

	return H;
}

