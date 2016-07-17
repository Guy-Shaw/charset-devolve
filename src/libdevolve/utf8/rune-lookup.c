#include <rune-table.h>

extern rune_table_t rune_table;

char *
rune_lookup(Rune r)
{
    Rune s, e;
    uint_t i;
    segment_t *segp;
    uint_t sz;

    sz = rune_table.nsegments;

    for (i = 0; i < sz; ++i) {
        segp = rune_table.segbase + i;
        s = segp->start;
        if (r < s) {
            return (NULL);
        }
        e = s + segp->sz;
        if (r < e) {
            return (segp->tr[r - s]);
        }
    }

    return (NULL);
}
