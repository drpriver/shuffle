#ifndef TERM_UTIL_H
#define TERM_UTIL_H

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

#endif
