#ifndef SWITCH_COMPAT_H
#define SWITCH_COMPAT_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef HAVE_STRDUP
static inline char *switch_strdup_fallback(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}
#ifndef strdup
#define strdup switch_strdup_fallback
#endif
#endif

#endif /* SWITCH_COMPAT_H */


