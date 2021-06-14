#ifndef KECCAK_H
#define KECCAK_H

#include <inttypes.h>

typedef struct state_array
{
	uint8_t w;
	uint32_t *array;
}
stateray_t;


int STATERAY_init(stateray_t*, uint8_t);
void STATERAY_free(stateray_t*);
void STATERAY_fill(stateray_t*, const uint8_t*);
uint8_t *STATERAY_getstring(const stateray_t*);


stateray_t *theta(const stateray_t*);

stateray_t *rho(const stateray_t*);

stateray_t *pi(const stateray_t*);

stateray_t *chi(const stateray_t*);

int rc(uint16_t);
stateray_t *iota(const stateray_t*, uint8_t);

stateray_t *Rnd(const stateray_t*, uint8_t);

uint8_t *keccak_p(uint16_t, uint8_t, const uint8_t*);
uint8_t *keccak_f(uint16_t, const uint8_t*);
uint8_t *keccak_c(uint16_t, const uint8_t*, uint32_t, uint32_t);

uint8_t *pad10star1(uint32_t, uint32_t, uint32_t*);


#endif

