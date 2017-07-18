/*
 * Filename: src/libdevolve/latin1/devolve-latin1.c
 * Project: charset-devolve
 * Brief: Devolve Latin1 8-bit character set down to 7-bit ASCII
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

#include <devolve.h>

extern char *program_path;
extern char *program_name;

extern bool verbose;
extern bool debug;

extern FILE *errprint_fh;
extern FILE *dbgprint_fh;

static void
fput_hex(int c, FILE *dstf)
{
    fprintf(dstf, "\\x%02x", c & 0xff);
}

#define latin1_table_base 0xa0

static char *
latin1_devolve_chr(int c)
{
    extern char latin1_table[];
    extern char *zstr_table[];

    static char cbuf[4];
    uint_t idx;
    uint_t bytecode;

    if (c > 0xff) {
        return (NULL);
    }
    idx = (c & 0xff) - latin1_table_base;
    bytecode = latin1_table[idx] & 0xff;
    if (bytecode <= 0x7f) {
        cbuf[0] = bytecode;
        cbuf[1] = '\0';
        return (cbuf);
    }
    else {
        return (zstr_table[bytecode - 0x80]);
    }
}


int
devolve_stream_latin1(fvh_t *fvp, FILE *dstf, unsigned int opt)
{
    size_t file_count_lines;
    size_t file_count_runes;
    size_t file_count_inval;
    size_t line_count_runes;

    FILE *srcf;
    int c;

    file_count_lines = 0;
    file_count_runes = 0;
    file_count_inval = 0;
    line_count_runes = 0;
    srcf = fvp->fh;
    fvp->flnr = 0;

    while ((c = getc(srcf)) != EOF) {
        char *ascii;

        if (c <= 0x7F) {
            if (c == '\n') {
                if (line_count_runes != 0) {
                    ++file_count_lines;
                    file_count_runes += line_count_runes;
                    line_count_runes = 0;
                }
                ++fvp->flnr;
            }
            fputc(c, dstf);
            continue;
        }

        ++line_count_runes;
        ascii = latin1_devolve_chr(c);
        if (ascii != NULL) {
            fputs(ascii, dstf);
        }
        else {
            fput_hex(c, dstf);
            ++file_count_inval;
        }
    }

    if (opt & OPT_SHOW_COUNTS || (opt & OPT_SHOW_8BIT && file_count_runes != 0)) {
        fprintf(stderr, "%s:\n", fvp->fname);
        fprintf(stderr, "%9zu 8-bit characters in entire file.\n",
            file_count_runes);
        fprintf(stderr, "%9zu lines containing any 8-bit characters.\n",
            file_count_lines);
        fprintf(stderr, "%9zu 8-bit characters that are not valid latin1.\n",
            file_count_inval);
    }

    return ((file_count_inval == 0) ? 0 : 1);
}
