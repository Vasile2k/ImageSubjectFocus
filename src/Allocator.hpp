#pragma once

/*
 * Just some basic definitions
 * Can be improved later if the situation requires it
 */

#include <cstdlib>

#define ISF_MALLOC(sz) malloc(sz)
#define ISF_REALLOC(p, newsz) realloc(p, newsz)
#define ISF_FREE(p) free(p)
