#include "conversion.h"

#include <stdlib.h>

int h2b(uint8_t *H, uint32_t m, uint32_t n)
{
	uint8_t j, bit, hold;
	uint32_t i;

	if(H == NULL) return -1;

	if(n > 8 * m || !n) n = 8 * m;


	for(i = 0; i < m && 8 * i + j < n; i++)
	{
		hold = 0;
		for(j = 0; j < 8 && 8 * i + j < n; j++)
		{
			bit = (H[i] >> j) & 1;
			hold |= bit << (7 - j);
		}

		H[i] = hold;
		j = 0;
	}

	return 0;
}

int b2h(uint8_t *S, uint32_t n)
{
	uint8_t bit, hold;
	uint32_t m, i;

	if(S == NULL) return -1;

	m = (n / 8) + (n % 8 != 0);

	hold = 0;
	for(i = 0; i < n; i++)
	{
		bit = (S[i / 8] >> (7 - (i % 8))) & 1;
		hold |= bit << (i % 8);

		if(i % 8 == 7)
		{
			S[i / 8] = hold;
			hold = 0;
		}
	}

	return 0;
}

