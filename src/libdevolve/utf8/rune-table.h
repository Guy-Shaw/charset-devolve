#ifndef RUNE_TABLE_H

#define RUNE_TABLE_H

#include <utf.h>

#define NULL 0

typedef unsigned int uint_t;

struct segment {
    uint_t start;
    uint_t sz;
    char **tr;
};

typedef struct segment segment_t;

struct rune_table {
    segment_t *segbase;
    uint_t nsegments;
};

typedef struct rune_table rune_table_t;

#endif /* RUNE_TABLE_H */
