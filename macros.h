#ifndef MACROS_H
#define MACROS_H

#if defined(__GNUC__) || defined(__clang__)
#define force_inline __attribute__((always_inline))
#else
#define force_inline
#endif

#ifdef __clang__
#define Nonnull(x) x _Nonnull
#define Nullable(x) x _Nullable
#define NullUnspec(x) x _Null_unspecified
#else
#define Nonnull(x) x
#define Nullable(x) x
#define NullUnspec(x) x
#endif

#endif
