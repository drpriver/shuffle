#ifndef david_macros_h
#define david_macros_h

// I've made a half-hearted attempt to get things to work with non-gcc,
// non-clang compilers. Oh well.

#ifndef warn_unused
#if defined(__GNUC__) || defined(__clang__)
#define warn_unused __attribute__ ((warn_unused_result))
#elif defined(_MSC_VER)
#define warn_unused _Check_return
#else
#error "No warn unused analogue"
#endif
#endif

#include <assert.h>
#include <stdbool.h>

#if defined(__GNUC__) || defined(__clang__)
#define force_inline __attribute__((always_inline))
#define never_inline __attribute__((noinline))
#else
#define force_inline
#define never_inline
#endif

#ifndef __cplusplus
#define and &&
#define or ||
#define not !
#endif

#ifndef __clang__
#define _Nonnull
#define _Nullable
#define _Null_unspecified
#endif
#define Nonnull(x) x _Nonnull
#define Nullable(x) x _Nullable
#define NullUnspec(x) x _Null_unspecified
typedef const char* _Null_unspecified const_c_string;
typedef char* _Null_unspecified c_string;

/*
   Force a compilation error if condition is true, but also produce a result
   (of value 0 and type size_t), so the expression can be used e.g. in a
   structure initializer (or where-ever else comma expressions aren't
   permitted).
*/
#ifndef WINDOWS
// this is weird... on windows this evaluates to 4, while on linux/mac this is 0
#define BUILD_BUG_IF(e) (sizeof(struct { int:-!!(e); }))
#define __must_be_array(a) \
 BUILD_BUG_IF(__builtin_types_compatible_p(typeof(a), typeof(&a[0])))
#else
#define __must_be_array(a) 0
#endif

#define arrlen(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

#if defined(__GNUC__) || defined(__clang__)
    #define PACKED __attribute__((__packed__))
    #ifndef WINDOWS
    #define SmallEnum enum __attribute__((__packed__))
        #if !defined(__clang__)
        #define FlagEnum enum
        #else
        #define FlagEnum enum __attribute__((flag_enum))
        #endif
    #else
    // leave SmallEnum undefined as this doesn't work on windows
    #define FlagEnum enum
    #endif
#else
// leave both undefined as things are going to be broken
#endif

#define toggle(x) (x = !x)

#ifdef DEBUG
#define unreachable() ({assert(0);__builtin_unreachable();})
#else
#define unreachable() __builtin_unreachable()
#endif

#define unhandled_error_condition(cond) assert(!(cond))

#if defined(__clang__)
#define nosan \
    __attribute__((no_sanitize("address"))) \
    __attribute__((no_sanitize("nullability"))) \
    __attribute__((no_sanitize("undefined")))
#define nosan_null __attribute__((no_sanitize("nullability")))
#elif defined(__GNUC__)
#define nosan \
    __attribute__((no_sanitize("address"))) \
    __attribute__((no_sanitize("undefined")))
#define nosan_null
#elif defined(__GNUC__)
#else
#define nosan
#define nosan_null
#endif

#if defined(__GNUC__) || defined(__clang__)
#define unimplemented() ({assert(!"This code not implemented yet.");__builtin_unreachable();})
#else
#define unimplemented() assert(!"This code not implemented yet.")
#endif

// TODO: breakpoints on arm
#define breakpoint()  asm("int $3")

#define GET_ARG_11(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
#define COUNT_MACRO_ARGS(...) GET_ARG_11( __VA_ARGS__, 10,9,8,7,6,5,4,3,2,1,I_CANNOT_SEE_ZERO_ARGS)

// These all use GNU statement expressions, so I'll have to address those later
#define Swap(x,y) ({ 	auto _tmp = x; \
		 	x = y;\
			y = _tmp;})

#define Max(a, b) ({ 	auto max_temp_a__ = (a);\
			auto max_temp_b__ = (b);\
			max_temp_a__ > max_temp_b__ ? max_temp_a__: max_temp_b__;})

#define Max_literal(a, literal) ({ auto _a = a;\
            auto _b = (typeof(_a)) literal;\
            _a > _b ? _a : _b;})

#define Min(a, b) ({ 	auto min_temp_a__ = (a);\
			auto min_temp_b__ = (b);\
			min_temp_a__ < min_temp_b__ ? min_temp_a__: min_temp_b__;})

#define Min_literal(a, literal) ({ auto _a = a;\
            auto _b = (typeof(_a)) literal;\
            _a < _b ? _a : _b;})

#define Abs(a) ({ auto _a = a;\
                    _a < 0 ? -_a : _a;})
#define Sign(x) ( {\
        auto _x = x;\
        (_x > 0) - (_x < 0);} )


#ifdef __clang__
#define typeof_member(Struct, m)\
    PushDiagnostic() \
    SuppressMissingBraces() \
    typeof(((Struct){0}).m)\
    PopDiagnostic()
#else
#define typeof_member(Struct, m)\
    typeof(((Struct){0}).m)
#endif

#define const_free(ptr) do{\
    PushDiagnostic(); \
    SuppressDiscardQualifiers(); \
    free(ptr);\
    PopDiagnostic(); \
    }while(0)

/*
 * Warning Suppression
 *
 * Pragmas to suppress warnings. Currently only supporting
 * clang, but using these macros means I can use the
 * compiler-specific pragmas.
 */
#ifdef __clang__
#define SuppressNullabilityComplete() _Pragma("clang diagnostic ignored \"-Wnullability-completeness\"")
#define SuppressUnusedFunction()  _Pragma("clang diagnostic ignored \"-Wunused-function\"")
#define SuppressCastQual()  _Pragma("clang diagnostic ignored \"-Wcast-qual\"")
#define SuppressDiscardQualifiers() _Pragma("clang diagnostic ignored \"-Wincompatible-pointer-types-discards-qualifiers\"")
#define SuppressMissingBraces()  _Pragma("clang diagnostic ignored \"-Wmissing-braces\"")
#define SuppressDoublePromotion() _Pragma("clang diagnostic ignored \"-Wdouble-promotion\"")
#define SuppressCastFunction()
#define PushDiagnostic() _Pragma("clang diagnostic push")
#define PopDiagnostic() _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define SuppressNullabilityComplete()
#define SuppressUnusedFunction()
#define SuppressCastQual() _Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#define SuppressDiscardQualifiers() _Pragma("GCC diagnostic ignored \"-Wdiscarded-qualifiers\"")
#define SuppressDoublePromotion() _Pragma("GCC diagnostic ignored \"-Wdouble-promotion\"")
#define SuppressMissingBraces()  _Pragma("GCC diagnostic ignored \"-Wmissing-braces\"")
#define SuppressCastFunction() _Pragma("GCC diagnostic ignored \"-Wcast-function-type\"")
#define PushDiagnostic() _Pragma("GCC diagnostic push")
#define PopDiagnostic() _Pragma("GCC diagnostic pop")
#else
#define SuppressNullabilityComplete()
#define SuppressUnusedFunction()
#define SuppressCastQual()
#define SuppressCastFunction()
#define SuppressMissingBraces()
#define SuppressDiscardQualifiers()
#define SuppressDoublePromotion()
#define PushDiagnostic()
#define PopDiagnostic()
#endif

/*
 * Allocator attributes
 *
 * MALLOC_FUNC: this function is malloc-like and the compiler
 *   is free to assume the pointer returned does not alias any
 *   other pointer. Note that realloc does not meet that criteria
 *   as it can fail and leave the original pointer valid.
 * ALLOCATOR_SIZE(N): the argument at the specified index
 *   (1-based) is the size of the storage that the returned
 *   pointer points to.
 */
#if defined(__GNUC__) || defined(__clang__)
#define ALLOCATOR_SIZE(N) __attribute__((alloc_size(N)))
#define MALLOC_FUNC __attribute__((malloc))
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
#else
#define ALLOCATOR_SIZE(...)
#define MALLOC_FUNC
#define likely(x) (x)
#define unlikely(x) (x)
#endif


/*
 * printf-like attributes
 */

#if defined(__GNUC__) || defined(__clang__)
#define printf_func(fmt_idx, vararg_idx) __attribute__((__format__ (__printf__, fmt_idx, vararg_idx)))
#else
#define printf_func(...)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define PURE __attribute__((pure))
#define CONST __attribute__((const))
#else
#define PURE
#define CONST
#endif

#if defined(__GNUC__) || defined(__clang__)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#endif
