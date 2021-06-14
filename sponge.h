#ifndef SPONGE_H
#define SPONGE_H

#include <inttypes.h>

typedef struct sponge_f
{
	uint8_t *(*f)(uint16_t, const uint8_t*);
	uint16_t b;
}
spng_f_t;

uint8_t *sponge(const spng_f_t*, uint8_t *(uint32_t, uint32_t, uint32_t*), uint32_t, const uint8_t*, uint32_t, uint32_t);

#endif

