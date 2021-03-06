//
// Copyright © 2021-2022, David Priver
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "rng.h"

#ifdef _WIN32

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <io.h>
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
static inline int stdin_is_interactive(void){
    return _isatty(STDIN_FILENO);
}
#else
#include <unistd.h>
#include <sys/ioctl.h>
static inline int stdin_is_interactive(void){
    return isatty(STDIN_FILENO);
}
#endif

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

typedef struct PointerArray {
    size_t capacity;
    void*_Null_unspecified* data;
    size_t count;
} PointerArray;

static inline PointerArray make_pointer_array(void);
static inline void push(PointerArray*array, void*_Null_unspecified p);
static inline void shuffle_pointers(RngState* rng, void*_Nullable* data, size_t count);
static inline void* memdup(const void*src, size_t length);

enum {
    F_NONE          = 0x0,
    F_SKIP_BLANKS   = 0x1,
    F_STOP_ON_BLANK = 0x2,
    F_PRINT_NEWLINE = 0x4,
};
static void get_lines(PointerArray* array, FILE* fp, unsigned flags);

static void print_help(const char* progname);
static void print_usage(const char* progname);

int
main(int argc, char** argv){
    unsigned flags = 0;
    PointerArray files = make_pointer_array();
    int read_options = 1;
    int args_are_lines = 0;
    int next_arg_is_arg = 0;
    int read_stdin = 0;
    int allow_replacement = 0;
    char* seed = NULL;
    char** arg_to_set = NULL;
    char* n_str = NULL;
    const char* name_of_arg_to_set = "(INTERNAL LOGIC ERROR)";
    size_t n = 0;
    PointerArray input = make_pointer_array();
    for(int i = 1; i < argc; i++){
        char* s = argv[i];
        if(args_are_lines){
            push(&input, s);
            continue;
        }
        if(next_arg_is_arg){
            assert(arg_to_set);
            *arg_to_set = argv[i];
            next_arg_is_arg--;
            continue;
        }
        if(s[0] == '-' && read_options){
            s++;
            char c;
            while((c = *s++)){
                switch(c){
                    case 'a':
                        if(next_arg_is_arg || args_are_lines){
                            fprintf(stderr, "More than one arg consuming argument provided: '%c', %s was set.\n", c, name_of_arg_to_set);
                            print_usage(argv[0]);
                            return 1;
                        }
                        args_are_lines = 1;
                        name_of_arg_to_set = "-a";
                        continue;
                    case 'n':
                        if(next_arg_is_arg || args_are_lines){
                            fprintf(stderr, "More than one arg consuming argument provided: '%c', %s was set.\n", c, name_of_arg_to_set);
                            print_usage(argv[0]);
                            return 1;
                        }
                        next_arg_is_arg = 1;
                        arg_to_set = &n_str;
                        name_of_arg_to_set = "-n";
                        continue;
                    case 'S':
                        if(next_arg_is_arg || args_are_lines){
                            fprintf(stderr, "More than one arg consuming argument provided: '%c', %s was set.\n", c, name_of_arg_to_set);
                            print_usage(argv[0]);
                            return 1;
                        }
                        next_arg_is_arg = 1;
                        arg_to_set = &seed;
                        name_of_arg_to_set = "-S";
                        continue;
                    case 's':
                        flags |= F_SKIP_BLANKS;
                        continue;
                    case 'i':
                        read_stdin = 1;
                        continue;
                    case 'h':
                        print_help(argv[0]);
                        return 0;
                    case 'b':
                        flags |= F_STOP_ON_BLANK;
                        continue;
                    case 'r':
                        allow_replacement = 1;
                        continue;
                    case '-':
                        read_options = 0;
                        if(*s != '\0'){
                            fprintf(stderr, "--long-arg options are not supported: '%s'\n", argv[i]);
                            print_usage(argv[0]);
                            return 1;
                        }
                        continue;
                    default:
                        fprintf(stderr, "Illegal option: '%c'\n", c);
                        print_usage(argv[0]);
                        return 1;
                }
            }
            continue;
        }
        FILE* fp = fopen(s, "r");
        if(!fp){
            fprintf(stderr, "cannot open '%s': %s\n", s, strerror(errno));
            return 1;
        }
        push(&files, fp);
    }
    if(next_arg_is_arg){
        fprintf(stderr, "Unexpected end of arguments. '%s' expected an argument afterwards\n", name_of_arg_to_set);
        return 1;
    }
    if(n_str){
        int n_val = atoi(n_str);
        if(n_val < 1){
            fprintf(stderr, "n must be an integer greater than 0, not '%s'\n", n_str);
            return 1;
        }
        n = n_val;
    }
    if(read_stdin || (!files.count && !input.count)){
        int interactive = stdin_is_interactive();
        unsigned f = flags;
        if(interactive){
            f |= F_STOP_ON_BLANK;
            f |= F_PRINT_NEWLINE;
        }
        else {
            f |= F_SKIP_BLANKS;
        }
        get_lines(&input, stdin, f);
    }
    for(size_t i = 0; i < files.count; i++){
        FILE* fp = files.data[i];
        get_lines(&input, fp, flags);
        // fclose(fp);
    }
    if(!input.count)
        return 0;
    RngState rng;
    if(seed)
        seed_rng_string(&rng, seed, strlen(seed));
    else
        seed_rng_auto(&rng);
    shuffle_pointers(&rng, input.data, input.count);
    if(!n)
        n = input.count;
    if(!allow_replacement){
        if(n > input.count)
            n = input.count;
        for(size_t i = 0; i < n; i++){
            puts(input.data[i]);
        }
    }
    else {
        for(size_t i = 0; i < n; i++){
            uint32_t index = bounded_random(&rng, input.count);
            const char* s = input.data[index];
            puts(s);
        }
    }
    return 0;
}

static
void
print_help(const char* progname){
    fprintf(stdout,
"%s: Shuffles lines, outputting them in a random order.\n"
"\n"
#define USAGE "usage: %s [-bhirs] [-S SEED] [-n N] [--] [file ...] [-a ARG ...]\n"
USAGE
"\n"
"Flags:\n"
"------\n"
"-b: Stop when the first blank line is encountered.\n"
"-h: Print this help and exit.\n"
"-i: Read lines from stdin (in addition to the input files).\n"
"-r: Allow repeats in the output (draw with replacement).\n"
"-s: Skip blank lines in files.\n"
"--: Interpret all following arguments as filenames\n"
"    so filenames starting with '-' can be read.\n"
"\n"
"Consuming Args (consumes next argument):\n"
"----------------------------------------\n"
"-a [ARG ...]: Treat the following arguments as input lines.\n"
"-n N:         Print no more than N output lines.\n"
"              Will not print more than the number of input lines,\n"
"              unless the -r flag is set.\n"
"-S SEED:      Seed the rng with the given string\n"
"              If not given, seeds via the system.\n"
"\n"
"If no filenames are listed or the -i flag is passed, %s will read\n"
"from stdin until the EOF is encounted (eg, ^D) or a blank line is inputted.\n"
, progname, progname, progname);
}

static
void
print_usage(const char* progname){
    fprintf(stderr, USAGE, progname);
}

static inline
PointerArray
make_pointer_array(void){
    PointerArray array;
    array.count = 0;
    array.capacity = 8;
    array.data = malloc(sizeof(*array.data)*array.capacity);
    if(!array.data){
        perror("Failed to make PointerArray: malloc");
        exit(1);
    }
    return array;
}

static inline
void
push(PointerArray* array, void*_Null_unspecified p){
    if(array->count >= array->capacity){
        array->capacity *= 2;
        array->data = realloc(array->data, sizeof(*array->data) * array->capacity);
        if(!array->data){
            perror("Failed to resize PointerArray: realloc");
            exit(1);
        }
    }
    array->data[array->count++] = p;
}

// fisher-yates shuffle
static inline
void
shuffle_pointers(RngState* rng, void*_Nullable*data, size_t count){
    if(count < 2)
        return;
    for(size_t i = 0; i < count; i++){
        size_t j = bounded_random(rng, count-i) + i;
        void* temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }
}

static inline
void*
memdup(const void* src, size_t length){
    void* p = malloc(length);
    if(!p){
        perror("Failed to memdup: malloc");
        exit(1);
    }
    memcpy(p, src, length);
    return p;
}

static
void
get_lines(PointerArray* array, FILE* fp, unsigned flags){
    char buff[8192];
    while(fgets(buff, sizeof(buff), fp)){
        size_t len = strlen(buff);
        if(len == 1){
            if(flags & F_STOP_ON_BLANK)
                return; // return instead of break to skip PRINT_NEWLINE
            if(flags & F_SKIP_BLANKS)
                continue;
        }
        if(!len) // I think this is impossible?
            continue;
        buff[len-1] = '\0';
        void* s = memdup(buff, len); // includes newly placed nul
        push(array, s);
    }
    if(ferror(fp)){
        fprintf(stderr, "Error while reading: %s\n", strerror(errno));
        exit(1);
    }
    if(flags & F_PRINT_NEWLINE)
        putchar('\n');
}
#ifdef __clang__
#pragma clang assume_nonnull end
#endif
