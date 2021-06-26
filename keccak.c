#include "keccak.h"

#include "sponge.h"

#include <stdlib.h>

static uint8_t log_2(uint32_t n)
{
	uint8_t p;
	uint32_t lsb = n & (-n);

	if(n == 0) return 0;

	p = 0;
	while(lsb > 1)
	{
		lsb >>= 1;
		p++;
	}

	return p;
}

uint8_t *pad10star1(uint32_t x, uint32_t m, uint32_t *bitlen)
{
	uint8_t *P;
	uint32_t j;

	if(x == 0) return NULL;

	j = x;
	while(j <= m + 2) j += x;

	j -= (m + 2);

	x = ((j + 2) / 8) + ((j + 2) % 8 > 0);

	*bitlen = j + 2;

	P = calloc(x, sizeof(uint8_t));

	P[0] |= 0x80;

	P[x - 1] |= 1 << (7 - ((j + 1) % 8)); /* TODO: check if this is right */

	return P;
}

static void set(stateray_t *state_array, uint8_t x, uint8_t y, uint8_t z, int bit)
{
	uint32_t *slice;

	if(y >= 5 || x >= 5 || z >= state_array->w) return;
	
	slice = &(state_array->array[z]);

	*slice &= ~(1 << (5 * y + x));

	*slice |= !!bit << (5 * y + x);
}

static uint8_t get(const stateray_t *state_array, uint8_t x, uint8_t y, uint8_t z)
{
	if(y >= 5 || x >= 5 || z >= state_array->w) return -1;


	return (state_array->array[z] >> (5 * y + x)) & 1;
}

int STATERAY_init(stateray_t *state_array, uint8_t w)
{
	state_array->w = w;

	state_array->array = calloc(w, sizeof(uint32_t));

	if(state_array->array == NULL) return 0;

	return 1;
}

void STATERAY_free(stateray_t *state_array)
{
	free(state_array->array);
}

void STATERAY_fill(stateray_t *state_array, const uint8_t *S)
{
	uint8_t x, y, z, bit;
	uint32_t c_i;

	for(y = 0; y < 5; y++)
	{
		for(x = 0; x < 5; x++)
		{
			for(z = 0; z < state_array->w; z++)
			{
				c_i = state_array->w * (5 * y + x) + z;

				bit = (S[c_i / 8] >> (7 - (c_i % 8))) & 1;

				set(state_array, x, y, z, bit);
			}
		}
	}
}

uint8_t *STATERAY_getstring(const stateray_t *state_array)
{
	uint8_t x, y, z, *string;
	uint32_t ibit, len, bitlen = state_array->w * 25;

	len = (bitlen / 8) + (bitlen % 8 > 0);

	string = calloc(len, sizeof(uint8_t));

	if(string == NULL) return NULL;

	ibit = 0;
	for(y = 0; y < 5; y++)
	{
		for(x = 0; x < 5; x++)
		{
			for(z = 0; z < state_array->w; z++)
			{
				string[ibit / 8] |= get(state_array, x, y, z) << (7 - (ibit % 8));

				ibit++;
			}
		}
	}

	return string;
}

stateray_t *theta(const stateray_t *A)
{
	uint8_t i, x, z, *C[5], *D[5];
	stateray_t *A_prime = malloc(sizeof(stateray_t));

	if(A_prime == NULL) return A_prime;

	for(i = 0; i < 5; i++)
	{
		C[i] = calloc(A->w, sizeof(uint8_t));

		if(C[i] == NULL)
		{
			for(--i; i != 0xFF; i--)
				free(C[i]);

			free(A_prime);

			return NULL;
		}
	}

	for(i = 0; i < 5; i++)
	{
		D[i] = malloc(A->w * sizeof(uint8_t));

		if(D[i] == NULL)
		{
			for(--i; i != 0xFF; i--)
				free(D[i]);

			for(i = 0; i < 5; i++)
				free(C[i]);

			free(A_prime);

			return NULL;
		}
	}

	
	/* Assign values of C[x,z] */
	for(x = 0; x < 5; x++)
	{
		for(z = 0; z < A->w; z++)
		{
			for(i = 0; i < 5; i++)
			{
				C[x][z] ^= get(A, x, i, z);
			}
		}
	}


	/* Assign values of D[x,z] */
	for(x = 0; x < 5; x++)
	{
		for(z = 0; z < A->w; z++)
		{
			D[x][z] = C[(x + 4) % 5][z] ^ C[(x + 1) % 5][(z + A->w - 1) % A->w];
		}
	}


	for(i = 0; i < 5; i++) free(C[i]);
	C[0] = malloc(1 * sizeof(uint8_t));
	

	if(!STATERAY_init(A_prime, A->w))
	{
		for(i = 0; i < 5; i++) free(D[i]);
		
		free(A_prime);

		return NULL;
	}


	/* Assign values in A' */
	for(x = 0; x < 5; x++)
		for(i = 0; i < 5; i++)
			for(z = 0; z < A->w; z++)
			{
				C[0][0] = D[x][z] ^ get(A, x, i, z);
				set(A_prime, x, i, z, C[0][0]);
			}

	free(C[0]);
	for(i = 0; i < 5; i++) free(D[i]);

	return A_prime;
}

stateray_t *rho(const stateray_t *A)
{
	uint8_t x, y, z, t;
	uint16_t width;

	stateray_t *A_prime = malloc(sizeof(stateray_t));
	if(A_prime == NULL) return NULL;

	if(!STATERAY_init(A_prime, A->w))
	{
		free(A_prime);
		return NULL;
	}

	/* A'[0,0,z] = A[0,0,z] for all z such that 0 <= z < w */
	for(z = 0; z < A->w; z++)
		set(A_prime, 0, 0, z, get(A, 0, 0, z));

	x = 1;
	y = 0;

	width = A->w * ((300 / A->w) + 1);

	for(t = 0; t <= 23; t++) /* For t from 0 to 23: */
	{
		/* A'[x,y,z] = A[x,y,(z - (t+1)(t+2)/2) % w for all z such that 0 <= z < w */
		for(z = 0; z < A->w; z++)
		{
			set(A_prime, x, y, z, get(A, x, y, (z + width - (((t + 1) * (t + 2)) / 2)) % A->w));
		}

		/* Let (x,y) = (y,(2x+3y) % 5) */
		z = y;
		y = (2 * x + 3 * y) % 5;
		x = z;
	}

	return A_prime;
}

stateray_t *pi(const stateray_t *A)
{
	uint8_t x, y, z;

	stateray_t *A_prime = malloc(sizeof(stateray_t));
	if(A_prime == NULL) return NULL;

	if(!STATERAY_init(A_prime, A->w))
	{
		free(A_prime);
		return NULL;
	}

	/* For all triples (x,y,z) such that 
	 * 0 <= x < 5,
	 * 0 <= y < 5, and
	 * 0 <= z < w,
	 * let A'[x,y,z] = A[(x+3y) % 5,x,z] */
	for(x = 0; x < 5; x++)
		for(y = 0; y < 5; y++)
			for(z = 0; z < A->w; z++)
				set(A_prime, x, y, z, get(A, (x + 3 * y) % 5, x, z));
	return A_prime;
}

stateray_t *chi(const stateray_t *A)
{
	uint8_t x, y, z, bit;

	stateray_t *A_prime = malloc(sizeof(stateray_t));
	if(A_prime == NULL) return NULL;

	if(!STATERAY_init(A_prime, A->w))
	{
		free(A_prime);
		return NULL;
	}

	for(x = 0; x < 5; x++)
	{
		for(y = 0; y < 5; y++)
		{
			for(z = 0; z < A->w; z++)
			{
				bit = get(A, (x + 1) % 5, y, z) ^ 1;
				bit *= get(A, (x + 2) % 5, y, z);
				bit ^= get(A, x, y, z);

				set(A_prime, x, y, z, bit);
			}
		}
	}

	return A_prime;
}

int rc(uint16_t t)
{
	uint8_t i, b1, b2;
	uint16_t R = 0x80; /* R = 10000000 */

	if(!(t % 255)) return 1;

	for(i = 0; i < t % 255; i++)
	{
		/* R[0] = R[0] ^ R[8] */
		b1 = R >> 8;
		b2 = R & 1;

		R |= (b1 ^ b2) << 8;


		/* R[4] = R[4] ^ R[8] */
		b1 = (R >> 4) & 1;
		/* b2 is still R[8] */

		R &= 0x1EF;

		R |= (b1 ^ b2) << 4;


		/* R[5] = R[5] ^ R[8] */
		b1 = (R >> 3) & 1;

		R &= 0x1F7;

		R |= (b1 ^ b2) << 3;


		/* R[6] = R[6] ^ R[8] */
		b1 = (R >> 2) & 1;

		R &= 0x1FB;

		R |= (b1 ^ b2) << 2;


		R >>= 1;
	}

	return R >> 7;
}

stateray_t *iota(const stateray_t *A, uint8_t i_r)
{
	uint8_t x, y, z, j;
	uint64_t RC;

	stateray_t *A_prime = malloc(sizeof(stateray_t));
	if(A_prime == NULL) return NULL;

	if(A->w > 64) return NULL;

	if(!STATERAY_init(A_prime, A->w))
	{
		free(A_prime);
		return NULL;
	}

	for(x = 0; x < 5; x++)
		for(y = 0; y < 5; y++)
			for(z = 0; z < A->w; z++)
				set(A_prime, x, y, z, get(A, x, y, z));

	/* RC = 0^w */
	RC = 0x0; /* When doing post-completion documentation the value of RC was set
		     to have all bits on and I don't know why. */

	/* For j from 0 to l, let RC[2^j - 1] = rc(j+7i_r) */
	for(j = 0; j <= log_2(A->w); j++)
	{
		RC |= rc(j + 7 * i_r) << ((1 << j) - 1);
	}

	/* For all z such that 0 <= z < w,
	 * let A'[0,0,z] = A'[0,0,z] ^ RC[z] */
	for(z = 0; z < A->w; z++)
	{
		/* use x to store bit values to make code easier on the eyes */
		x = get(A_prime, 0, 0, z) ^ ((RC >> z) & 1);
		set(A_prime, 0, 0, z, x);
	}

	return A_prime;
}

stateray_t *Rnd(const stateray_t *state_array, uint8_t i_r)
{
	stateray_t *A;
	stateray_t *A_prime;

	A = theta(state_array);

	A_prime = rho(A);
	STATERAY_free(A);
	free(A);
	A = A_prime;

	A_prime = pi(A);
	STATERAY_free(A);
	free(A);
	A = A_prime;

	A_prime = chi(A);
	STATERAY_free(A);
	free(A);
	A = A_prime;

	A_prime = iota(A, i_r);
	STATERAY_free(A);
	free(A);
	
	return A_prime;
}

uint8_t *keccak_p(uint16_t b, uint8_t n_r, const uint8_t *S)
{
	uint8_t l, i_r, *S_prime;
	stateray_t *A = malloc(sizeof(stateray_t));
	stateray_t *A_prime;

	if(A == NULL || n_r == 0) return NULL;

	if(b != 25 && b != 50 && b != 100 && b != 200 && b != 400 && b != 800 && b != 1600) return NULL;

	l = log_2(b / 25);

	if(n_r > 12 + 2 * l) n_r = 12 + 2 * l;

	if(!STATERAY_init(A, b / 25))
	{
		free(A);
		return NULL;
	}

	STATERAY_fill(A, S);

	for(i_r = (12 + 2 * l) - n_r; i_r < 12 + 2 * l; i_r++)
	{
		A_prime = Rnd(A, i_r);

		STATERAY_free(A);
		free(A);

		A = A_prime;
	}

	S_prime = STATERAY_getstring(A);

	STATERAY_free(A);
	free(A);

	return S_prime;
}

uint8_t *keccak_f(uint16_t b, const uint8_t *S)
{
	uint8_t l;

	if(b != 25 && b != 50 && b != 100 && b != 200 && b != 400 && b != 800 && b != 1600) return NULL;

	l = log_2(b / 25);

	return keccak_p(b, 12 + 2 * l, S);
}

uint8_t *keccak_c(uint16_t c, const uint8_t *N, uint32_t n_bitlen, uint32_t d)
{
	spng_f_t keccak_f_1600;

	if(c >= 1600) return NULL;

	keccak_f_1600.f = &keccak_f;
	keccak_f_1600.b = 1600;

	return sponge(&keccak_f_1600, &pad10star1, 1600 - c, N, n_bitlen, d);
}

