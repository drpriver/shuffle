//
// This was altered from pcg_basic.c. Modifications are public domain.
// The license is below:
//
/*
 * PCG Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *       http://www.pcg-random.org
 */

//
// Modifications by D. Priver.
// Modifications by D. Priver are released into the public domain.
// 
// seed_rng_auto is original
// bounded_random was altered to use fast_reduce which gave a nice speed boost
// seed_rng_string is original
// Basically everything was renamed.
//
#ifndef RNG_H
#define RNG_H
// size_t
#include <stddef.h>
// uint64_t, uint32_t
#include <stdint.h>

#ifdef __linux__
// for getrandom
#include <sys/random.h>
#endif

#ifdef __APPLE__
// for arc4random_buf
#include <stdlib.h>
#endif

#include <assert.h>

#ifdef __clang__
#pragma clang assume_nonnull begin
#else
#ifndef _Null_unspecified
#define _Null_unspecified
#endif
#ifndef _Nullable
#define _Nullable
#endif
#endif

#ifndef force_inline
#if defined(__GNUC__) || defined(__clang__)
#define force_inline __attribute__((always_inline))
#else
#define force_inline
#endif
#endif

static inline
uint64_t
hashbytes(const void* vp, size_t length);

typedef struct RngState {
    uint64_t state;
    uint64_t inc;
} RngState;

//
// Produces a uniform random u32.
//
static inline
uint32_t
rng_random32(RngState* rng){
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = (uint32_t) ( ((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}


//
// Seeds the rng with the given values.
//
static inline
void
seed_rng_fixed(RngState* rng, uint64_t initstate, uint64_t initseq){
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    rng_random32(rng);
    rng->state += initstate;
    rng_random32(rng);
}


//
// Seeds the rng using os APIs when available.
// If not available, uses rdseed on x64.
//
static inline
void
seed_rng_auto(RngState* rng){
    _Static_assert(sizeof(unsigned long long) == sizeof(uint64_t), "");
    // spurious warnings on some platforms about unsigned long longs and unsigned longs,
    // so, use the unsigned long long.
    unsigned long long initstate;
    unsigned long long initseq;
#ifdef __APPLE__
    arc4random_buf(&initstate, sizeof(initstate));
    arc4random_buf(&initseq, sizeof(initseq));
#elif defined(__linux__)
    // Too lazy to check for error.  Man page says we can't get
    // interrupted by signals for such a small buffer. and we're
    // not requesting nonblocking.
    ssize_t read;
    read = getrandom(&initstate, sizeof(initstate), 0);
    read = getrandom(&initseq, sizeof(initseq), 0);
    (void)read;
#elif defined(__GNUC__) || defined(__clang__)
    // Idk how to get random numbers on windows.
    // Just use the intel instruction.
    //
    // Idk what the intrinsic on MSVC is.
    __builtin_ia32_rdseed64_step(&initstate);
    __builtin_ia32_rdseed64_step(&initseq);
#else
#error "Need to specify a way to seed an rng with this compiler or platform"
#endif
    seed_rng_fixed(rng, initstate, initseq);
}

//
// Seeds the rng by hashing the given string data.
//
static inline
void
seed_rng_string(RngState* rng, const char* restrict str, size_t len){
    uint64_t s = hashbytes(str, len);
    uint64_t s2 = s * 1087;
    seed_rng_fixed(rng, s, s2);
}

// from
// https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
static inline
force_inline
uint32_t
fast_reduce(uint32_t x, uint32_t N){
    return ((uint64_t)x * (uint64_t)N) >> 32;
}

//
// Returns a random u32 in the range of [0, bound).
//
static inline
uint32_t
bounded_random(RngState* rng, uint32_t bound){
    uint32_t threshold = -bound % bound;
    // bounded loop to catch unitialized rng errors
    for(size_t i = 0 ; i < 10000; i++){
        uint32_t r = rng_random32(rng);
        if(r >= threshold){
            uint32_t temp = fast_reduce(r, bound);
            // auto temp = r % bound;
            return temp;
        }
    }
    assert(0);
#if defined(__GNUC__) || defined(__clang__)
    __builtin_unreachable();
#else
    return 0;
#endif
}

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
murmur3_32(const uint8_t* key, size_t len, uint32_t seed){
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
hashbytes(const void* vp, size_t length){
    // FIXME: this should use a 64 bit hash, not a 32 bit hash.
    return murmur3_32(vp, length, 1107845655llu);
}

#ifdef __clang__
#pragma clang assume_nonnull end
#endif

#endif
