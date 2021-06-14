#ifndef SHA3_H
#define SHA3_H

#include <inttypes.h>

uint8_t *sha3_224(const char*, uint32_t);
uint8_t *sha3_256(const char*, uint32_t);
uint8_t *sha3_384(const char*, uint32_t);
uint8_t *sha3_512(const char*, uint32_t);

uint8_t *shake_128(const char*, uint32_t, uint32_t);
uint8_t *shake_256(const char*, uint32_t, uint32_t);

#endif

