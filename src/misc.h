#ifndef _FIG_MISC_H_
#define _FIG_MISC_H_

#include <stdio.h>
#include <stdlib.h>

#if !defined(NDEBUG)
#   define _FIG_DEBUG
#endif

#define PROG_NAME "Fig"

#define fig_warn(message) \
    fprintf(stderr, "[" PROG_NAME "] warning: %s\n", message);

#define fig_panic(message) \
    { \
	    fprintf(stderr, "[" PROG_NAME "] error: %s\n", message); \
	    exit(EXIT_FAILURE); \
    }

#endif /* _FIG_MISC_H_ */
