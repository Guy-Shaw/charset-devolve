/*
 * Filename: src/libdevolve/devolve-utf8.c
 * Project: charset-devolve
 * Brief: Devolve utf8 Runes down to 7-bit ASCII
 *
 * Copyright (C) 2016 Guy Shaw
 * Written by Guy Shaw <gshaw@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define IMPORT_FVH
#include <cscript.h>
#include <utf.h>

typedef size_t index_t;

extern char *rune_lookup(Rune);

extern char *program_path;
extern char *program_name;

extern bool verbose;
extern bool debug;

extern FILE *errprint_fh;
extern FILE *dbgprint_fh;

size_t count_lines;
size_t count_runes;

static Rune
getRune(FILE *f)
{
    Rune r;
    int c;
    char char_buf[UTFmax + 1];
    index_t i;

    i = 0;
    while (i < sizeof (char_buf)) {
        c = getc(f);
        if (c == EOF) {
            return ((Rune)EOF);
        }
        char_buf[i] = c;
        ++i;
        if (fullrune(char_buf, i)) {
            char_buf[i] = '\0';
            chartorune(&r, char_buf);
            return (r);
        }
    }
    return (Runeerror);
}

static void
fputRune(Rune r, FILE *f)
{
    char char_buf[UTFmax + 1];
    size_t sz;
    size_t i;

    fputc('*', f);
    fprintf(f, "U+%04x", r);
    fputc('=', f);
    sz = runetochar(char_buf, &r);
    for (i = 0; i < sz; ++i) {
        fprintf(f, "\\x%02x", char_buf[i] & 0xff);
    }
    fputc('*', f);
}

int
devolve_stream_utf8(fvh_t *fvp, FILE *dstf)
{
    FILE *srcf;
    Rune r;
    size_t count_runes_this_line;

    count_lines = 0;
    count_runes = 0;
    count_runes_this_line = 0;
    srcf = fvp->fh;
    fvp->flnr = 0;
    while ((r = getRune(srcf)) != (Rune) EOF) {
        char *ascii;

        if (r <= 0x7F) {
            if (r == '\n') {
                if (count_runes_this_line != 0) {
                    count_runes += count_runes_this_line;
                    ++count_lines;
                    count_runes_this_line = 0;
                }
                ++fvp->flnr;
            }
            fputc(r, dstf);
            continue;
        }
        ascii = rune_lookup(r);
        if (ascii != NULL) {
            fputs(ascii, dstf);
        }
        else {
            fputRune(r, dstf);
            ++count_runes_this_line;
        }
    }

    return (0);
}
