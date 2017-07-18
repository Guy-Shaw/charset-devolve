/*
 * Filename: src/libdevolve/utf8/devolve-utf8.c
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

#include <devolve.h>

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
getRune(FILE *f, int c1)
{
    Rune r;
    int c;
    char char_buf[UTFmax + 1];
    size_t i;

    i = 0;
    if (c1 != 0) {
        char_buf[i] = c1;
        ++i;
    }

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
    if (r == Runeerror) {
        fputs("*BAD*", f);
        return;
    }

    fputc('*', f);
    if ((r >= 0x80 && r <= 0xC1) || (r >= 0xF5 && r <= 0xFF)) {
        // This is not valid for the first byte of a UTF-8 rune
        fprintf(f, "%02x", r);
    }
    else {
        // The first byte is valid, but for some reason the entire
        // UTF-8 is either invalid OR it is merely not in the
        // Devolve-Unicode-to-ASCII table.

        char char_buf[UTFmax + 1];
        size_t sz;
        size_t i;

        fprintf(f, "U+%04x", r);
        fputc('=', f);
        sz = runetochar(char_buf, &r);
        for (i = 0; i < sz; ++i) {
            fprintf(f, "\\x%02x", char_buf[i] & 0xff);
        }
    }
    fputc('*', f);
}

/*
 * The input stream is read one byte at a time.
 * Bytes that are the start of a UTF-8 multi-byte code-point
 * get read by getRune() which advances as many bytes as are
 * needed to read in a full rune.
 *
 * We could just read full runes at a time, without first probing
 * (and consuming) the first byte.  That would work.  But, in case
 * of any first byte that is invalid, we want to report that as a
 * separate kind of error, and we want to recover by advancing only
 * one byte.  But getRune() can advance more than one byte, and then
 * report an error.
 *
 * Besides, we assume that ASCII characters (0 .. 0x7F) is the common
 * case.
 *
 */

int
devolve_stream_utf8(fvh_t *fvp, FILE *dstf, unsigned int opt)
{
    size_t cnt_8bit;
    size_t cnt_runes;
    size_t cnt_inval;
    size_t cnt_lines;
    size_t cnt_lines_with_8bit;
    size_t cnt_lines_with_runes;
    size_t cnt_lines_with_inval;
    size_t cnt_runes_this_line;
    size_t cnt_inval_this_line;

    FILE *srcf;
    int c;

    cnt_runes = 0;
    cnt_inval = 0;
    cnt_lines = 0;
    cnt_lines_with_8bit = 0;
    cnt_lines_with_runes = 0;
    cnt_lines_with_inval = 0;
    cnt_runes_this_line = 0;
    cnt_inval_this_line = 0;

    srcf = fvp->fh;
    fvp->flnr = 0;
    while (true) {
        c = getc(srcf);
        if (c == EOF || c == '\n') {
            ++cnt_lines;
            cnt_runes += cnt_runes_this_line;
            cnt_inval += cnt_inval_this_line;
            if (cnt_runes_this_line != 0) {
                ++cnt_lines_with_runes;
            }
            if (cnt_inval_this_line!= 0) {
                ++cnt_lines_with_inval;
            }
            if (cnt_runes_this_line != 0 || cnt_inval_this_line != 0) {
                ++cnt_lines_with_8bit;
            }
            cnt_runes_this_line = 0;
            cnt_inval_this_line = 0;
            ++fvp->flnr;
        }

        if (c == EOF) {
            break;
        }

        if (c <= 0x7F) {
            fputc(c, dstf);
        }
        else if (c <= 0xC0) {
            Rune r;
            r = (Rune)c;
            fputRune(r, dstf);
            ++cnt_inval_this_line;
        }
        else {
            char *ascii;
            Rune r;

            r = getRune(srcf, c);
            if (opt & OPT_SOFT_HYPHENS && r == 0x00AD) {
                ascii = "-";
            }
            else {
                ascii = rune_lookup(r);
            }
            if (ascii != NULL) {
                fputs(ascii, dstf);
                ++cnt_runes_this_line;
            }
            else {
                fputRune(r, dstf);
                ++cnt_inval_this_line;
            }
        }
    }

    cnt_8bit = cnt_runes + cnt_inval;
    if (opt & OPT_SHOW_COUNTS || (opt & OPT_SHOW_8BIT && cnt_8bit != 0)) {
        fprintf(stderr, "File: '%s':\n", fvp->fname);
        fprintf(stderr, "%9zu lines in file.\n",
            cnt_lines);
        fprintf(stderr, "%9zu non-ascii bytes (>= 0x80) in entire file.\n",
            cnt_8bit);
        fprintf(stderr, "%9zu UTF-8 runes in entire file.\n",
            cnt_runes);
        fprintf(stderr, "%9zu Invalid runes in entire file.\n",
            cnt_inval);
        fprintf(stderr, "%9zu lines containing any non-ascii bytes.\n",
            cnt_lines_with_8bit);
        fprintf(stderr, "%9zu lines containing any UTF-8 runes.\n",
            cnt_lines_with_runes);
        fprintf(stderr, "%9zu lines containing any invalid runes.\n",
            cnt_lines_with_inval);
    }

    return ((cnt_inval == 0) ? 0 : 1);
}
