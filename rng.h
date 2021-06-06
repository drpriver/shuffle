#ifndef RNG_H
#define RNG_H
// size_t
#include <stddef.h>
// uint64_t, uint32_t
#include <stdint.h>
#include <assert.h>
#include "macros.h"
#include "hashbytes.h"

typedef struct RngState {
    uint64_t state;
    uint64_t inc;
} RngState;

static inline
uint32_t
rng_random32(Nonnull(RngState*) rng){
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = (uint32_t) ( ((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

static inline
void
seed_rng_fixed(Nonnull(RngState*) rng, uint64_t initstate, uint64_t initseq){
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    rng_random32(rng);
    rng->state += initstate;
    rng_random32(rng);
}


#ifdef __linux__
#include <sys/random.h>
#endif
static inline
void
seed_rng_auto(Nonnull(RngState*) rng){
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
#else
    // Idk how to get random numbers on windows.
    // Just use the intel instruction.
    __builtin_ia32_rdseed64_step(&initstate);
    __builtin_ia32_rdseed64_step(&initseq);
#endif
    seed_rng_fixed(rng, initstate, initseq);
}

static inline
void
string_seed_rng(Nonnull(RngState*) rng, Nonnull(const char*) restrict str, size_t len){
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

static inline
uint32_t
bounded_random(Nonnull(RngState*) rng, uint32_t bound){
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
#if defined(__GNUC__)
    __builtin_unreachable();
#endif
}

#endif
