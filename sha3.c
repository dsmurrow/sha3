#include "sha3.h"

#include "conversion.h"
#include "keccak.h"

#include <stdlib.h>

static uint8_t *sha3(uint32_t c, const uint8_t *M, uint32_t m_len, uint32_t d)
{
	uint8_t *new_m;
	uint32_t i;
	uint8_t *hash;

	if(c != d * 2 && d != 224 && d != 256 && d != 384 && d != 512) return NULL;

	new_m = malloc((m_len + 1) * sizeof(uint8_t));
	if(new_m == NULL) return NULL;

	for(i = 0; i < m_len; i++) new_m[i] = M[i];
	new_m[m_len] = 0x40;

	h2b(new_m, m_len, m_len * 8);

	hash = keccak_c(c, new_m, m_len * 8 + 2, d);

	free(new_m);

	b2h(hash, d);

	return hash;
}

uint8_t *sha3_224(const uint8_t *M, uint32_t m_len)
{
	return sha3(448, M, m_len, 224);
}

uint8_t *sha3_256(const uint8_t *M, uint32_t m_len)
{
	return sha3(512, M, m_len, 256);
}

uint8_t *sha3_384(const uint8_t *M, uint32_t m_len)
{
	return sha3(768, M, m_len, 384);
}

uint8_t *sha3_512(const uint8_t *M, uint32_t m_len)
{
	return sha3(1024, M, m_len, 512);
}


static uint8_t *shake(uint32_t c, const uint8_t *M, uint32_t m_len, uint32_t d)
{
	uint8_t *new_m, *hash;
	uint32_t i;

	if(c != 256 && c != 512) return NULL;

	new_m = malloc((m_len + 1) * sizeof(uint8_t));
	if(new_m == NULL) return NULL;

	for(i = 0; i < m_len; i++) new_m[i] = M[i];
	new_m[m_len] = 0xF0;

	h2b(new_m, m_len, m_len * 8);

	hash = keccak_c(c, new_m, m_len * 8 + 4, d);

	free(new_m);

	b2h(hash, d);

	return hash;
}

uint8_t *shake_128(const uint8_t *M, uint32_t m_len, uint32_t d)
{
	return shake(256, M, m_len, d);
}

uint8_t *shake_256(const uint8_t *M, uint32_t m_len, uint32_t d)
{
	return shake(512, M, m_len, d);
}
