#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "term_util.h"
#include "macros.h"
#include "rng.h"

typedef struct PointerArray {
    size_t capacity;
    Nonnull(NullUnspec(void*)*) data;
    size_t count;
} PointerArray;

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

// fisher-yates shuffle
static inline
void
shuffle_pointers(Nonnull(RngState*)rng, Nonnull(Nullable(void*)*)data, size_t count){
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
void
push(Nonnull(PointerArray*)array, NullUnspec(void*) p){
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

static inline
Nonnull(void*)
memdup(Nonnull(const void*)src, size_t length){
    void* p = malloc(length);
    if(!p){
        perror("Failed to memdup: malloc");
        exit(1);
        }
    memcpy(p, src, length);
    return p;
    }

enum {
    F_NONE          = 0x0,
    F_SKIP_BLANKS   = 0x1,
    F_STOP_ON_BLANK = 0x2,
    F_PRINT_NEWLINE = 0x4,
};

static
void
get_lines(Nonnull(PointerArray*) array, Nonnull(FILE*)fp, unsigned flags){
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
        len++; // include nul
        void* s = memdup(buff, len);
        push(array, s);
        }
    if(ferror(fp)){
        fprintf(stderr, "Error while reading: %s\n", strerror(errno));
        exit(1);
        }
    if(flags & F_PRINT_NEWLINE)
        putchar('\n');
    }

void
print_help(const char* progname){
    fprintf(stdout,
"%s: Shuffles lines, outputting them in a random order.\n"
"\n"
"usage: %s [-bhis] [-S SEED] [-n N] [--] [file ...]\n"
"\n"
"Flags:\n"
"------\n"
"-b: Stop when the first blank line is encountered.\n"
"-h: Print this help and exit.\n"
"-i: Read lines from stdin (in addition to the input files).\n"
"-s: Skip blank lines in files.\n"
"--: Interpret all following arguments as filenames\n"
"    so filenames starting with '-' can be read.\n"
"\n"
"Consuming Args (consumes next argument):\n"
"----------------------------------------\n"
"-S SEED: Seed the rng with the given string\n"
"         If not given, seeds via the system.\n"
"-n N:    Print no more than N output lines.\n"
"         Will not print more than the number of input lines.\n"
"\n"
"If no filenames are listed or the -i flag is passed, %s will read\n"
"from stdin until the EOF is encounted (eg, ^D) or a blank line is inputted.\n"
, progname, progname, progname);
    }

void
print_usage(const char* progname){
    fprintf(stderr, "usage: %s [-bhis] [-S SEED] [-n N] [--] [file ...]\n", progname);
    }

int
main(int argc, char** argv){
    unsigned flags = 0;
    PointerArray files = make_pointer_array();
    int read_options = 1;
    int next_arg_is_arg = 0;
    int read_stdin = 0;
    char* seed = NULL;
    char** arg_to_set = NULL;
    char* n_str = NULL;
    const char* name_of_arg_to_set = "(INTERNAL LOGIC ERROR)";
    size_t n = 0;
    for(int i = 1; i < argc; i++){
        char* s = argv[i];
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
                    case 'n':
                        if(next_arg_is_arg){
                            fprintf(stderr, "More than one arg consuming argument provided: '%c'\n", c);
                            print_usage(argv[0]);
                            return 1;
                            }
                        next_arg_is_arg = 1;
                        arg_to_set = &n_str;
                        name_of_arg_to_set = "-n";
                        continue;
                    case 'S':
                        if(next_arg_is_arg){
                            fprintf(stderr, "More than one arg consuming argument provided: '%c'\n", c);
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
    PointerArray input = make_pointer_array();
    if(!files.count || read_stdin){
        int interactive = isatty(STDIN_FILENO);
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
        string_seed_rng(&rng, seed, strlen(seed));
    else
        seed_rng_auto(&rng);
    shuffle_pointers(&rng, input.data, input.count);
    if(!n)
        n = input.count;
    if(n > input.count)
        n = input.count;
    for(size_t i = 0; i < n; i++){
        // fputs doesn't append a newline.
        fputs(input.data[i], stdout);
        }
    return 0;
    }
