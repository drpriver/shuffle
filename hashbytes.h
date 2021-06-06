#ifndef HASHBYTES_H
#define HASHBYTES_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "macros.h"

// cut'n'paste from the wikipedia page on murmur hash
static inline
force_inline
uint32_t
murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}
static inline
force_inline
uint32_t
murmur3_32(Nonnull(const uint8_t*) key, size_t len, uint32_t seed){
	uint32_t h = seed;
    uint32_t k;
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    h ^= murmur_32_scramble(k);
    /* Finalize. */
	h ^= len;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

static inline
uint64_t
hashbytes(Nonnull(const void*) vp, size_t length){
    // FIXME: this should use a 64 bit hash, not a 32 bit hash.
    return murmur3_32(vp, length, 1107845655llu);
}

#endif
