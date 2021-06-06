#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "term_util.h"
#include "macros.h"
#include "rng.h"
typedef struct String {
    size_t length;
    Nonnull(const char*) text; // not guaranteed to be nul-terminated
} String;

typedef struct StringArray {
    size_t capacity;
    Nonnull(String*) data;
    size_t count;
} StringArray;

static inline
StringArray
make_string_array(void){
    StringArray array;
    array.count = 0;
    array.capacity = 8;
    array.data = malloc(sizeof(*array.data)*array.capacity);
    if(!array.data){
        perror("Failed to make StringArray: malloc");
        exit(1);
        }
    return array;
}

// fisher-yates shuffle
static inline
void
shuffle_strings(Nonnull(RngState*)rng, Nonnull(String*)data, size_t count){
    if(count < 2)
        return;
    for(size_t i = 0; i < count; i++){
        size_t j = bounded_random(rng, count-i) + i;
        String temp = data[i];
        data[i] = data[j];
        data[j] = temp;
        }
    }

static inline
void
push(Nonnull(StringArray*)array, String s){
    if(array->count >= array->capacity){
        array->capacity *= 2;
        array->data = realloc(array->data, sizeof(*array->data) * array->capacity);
        if(!array->data){
            perror("Failed to resize StringArray: realloc");
            exit(1);
            }
        }
    array->data[array->count++] = s;
    }

static inline
Nonnull(void*)
memdup(Nonnull(void*)src, size_t length){
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
get_lines(Nonnull(StringArray*) array, Nonnull(FILE*)fp, unsigned flags){
    char buff[8192];
    while(fgets(buff, sizeof(buff), fp)){
        size_t len = strlen(buff);
        if(len == 1){
            if(flags & F_STOP_ON_BLANK)
                // return instead of break to skip PRINT_NEWLINE
                return;
            if(flags & F_SKIP_BLANKS)
                continue;
            }
        if(!len) // I think this is impossible?
            continue;
        if(buff[len-1] == '\n')
            len--; // remove newline
        String s = {
            .text = memdup(buff, len),
            .length = len,
            };
        push(array, s);
        }
    if(flags & F_PRINT_NEWLINE)
        putchar('\n');
    }

int main(int argc, char** argv){
    StringArray input = make_string_array();
    if(argc < 2){
        int interactive = isatty(STDIN_FILENO);
        unsigned flags = interactive? (F_STOP_ON_BLANK|F_PRINT_NEWLINE) : F_SKIP_BLANKS;
        get_lines(&input, stdin, flags);
        }
    else {
        for(int i = 1; i < argc; i++){
            FILE* fp = fopen(argv[i], "r");
            if(!fp){
                fprintf(stderr, "Cannot open '%s': %s\n", argv[i], strerror(errno));
                return 1;
                }
            get_lines(&input, fp, F_NONE);
            fclose(fp);
            }
        }
    if(!input.count)
        return 0;
    RngState rng;
    seed_rng_auto(&rng);
    shuffle_strings(&rng, input.data, input.count);
    for(size_t i = 0; i < input.count; i++){
        String s = input.data[i];
        printf("%.*s\n", (int)s.length, s.text);
        }
    return 0;
    }
