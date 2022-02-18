#ifndef SHA3_H
#define SHA3_H

#include <inttypes.h>

uint8_t *sha3_224(uint8_t*, uint32_t);
uint8_t *sha3_256(uint8_t*, uint32_t);
uint8_t *sha3_384(uint8_t*, uint32_t);
uint8_t *sha3_512(uint8_t*, uint32_t);

uint8_t *shake_128(uint8_t*, uint32_t, uint32_t);
uint8_t *shake_256(uint8_t*, uint32_t, uint32_t);

#endif

