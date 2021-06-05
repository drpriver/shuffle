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
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#endif
