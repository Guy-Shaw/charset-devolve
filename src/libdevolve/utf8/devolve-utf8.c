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
#include <string.h>
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

/*
 * Get a full UTF8 rune.
 * A single byte might already have been read.
 * So, we supply the value of the first byte, if any,
 * here, rather than doing ungetc(),
 * or some other character peeking / backtracking mechanism.
 *
 * Keep reading one character at a time,
 * until we get a full rune or an error.
 *
 */

static Rune
getRune(FILE *f, int c1, size_t *col)
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
        ++(*col);
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

/*
 * Decode a UTF-8 Rune as hexadecimal reresentation.
 *   1) as U+%04x and also,
 *   2) a series of raw hexadecimal bytes of the form \x%02x
 */
static char *
rune_to_hex_r(char *dst, size_t sz, Rune r)
{
    char char_buf[UTFmax + 1];
    char *dp;
    size_t rsz;
    size_t i;

    dp = dst;
    if (dst + sz - dp < 7) {
        abort();
    }
    sprintf(dp, "U+%04x=", r);
    dp += 7;

    rsz = runetochar(char_buf, &r);
    for (i = 0; i < rsz; ++i) {
        if (dst + sz - dp < 4) {
            abort();
        }
        sprintf(dp, "\\x%02x", char_buf[i] & 0xff);
        dp += 4;
    }
    *dp = '\0';
    return (dst);
}

static char *
rune_to_hex(Rune r)
{
    static char xdcode_rune[32];
    return (rune_to_hex_r((char *) &xdcode_rune, sizeof (xdcode_rune), r));
}

/*
 * If a rune was not ASCII, and could not be devolved to ASCII,
 * then we emit a representatrion of the non-translated rune,
 * or the offending non-utf8 sequence of bytes.
 *
 * The current source lines number and column are used solely
 * for the purpose of trace messages.
 */

static void
fputBadRune(Rune r, FILE *f, size_t lnr, size_t col, unsigned int opt)
{
    char dcode_bad[32];
    char *dp;

    dp = dcode_bad;
    *dp++ = '*';

    if (r == Runeerror) {
        strcpy(dp, "BAD");
        dp += 3;
    }
    else if ((r >= 0x80 && r <= 0xC1) || (r >= 0xF5 && r <= 0xFF)) {
        // This is not valid for the first byte of a UTF-8 rune
        sprintf(dp, "%02x", r);
        dp += 2;
    }
    else {
        // The first byte is valid, but for some reason the entire
        // UTF-8 is either invalid OR it is merely not in the
        // Devolve-Unicode-to-ASCII table.

        size_t rlen;

        rlen = (char *) &dcode_bad + sizeof (dcode_bad) - dp - 1;
        rune_to_hex_r(dp, rlen, r);
        while (*dp) {
            ++dp;
        }
    }
    *dp++ = '*';
    *dp = '\0';
    fputs(dcode_bad, f);
    if (opt & OPT_TRACE_ERRORS) {
        fprintf(stderr, " line #%zu, col #%zu, %s\n", lnr, col, dcode_bad);
    }
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
    size_t col;

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
    col = 0;
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
            col = 0;
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
            fputBadRune(r, dstf, fvp->flnr, col, opt);
            ++cnt_inval_this_line;
        }
        else {
            char *ascii;
            Rune r;

            r = getRune(srcf, c, &col);
            if (opt & OPT_SOFT_HYPHENS && r == 0x00AD) {
                ascii = "-";
            }
            else {
                ascii = rune_lookup(r);
            }
            if (ascii != NULL) {
                fputs(ascii, dstf);
                if (opt & OPT_TRACE_CONV) {
                    fprintf(stderr, "    line #%zu, col #%zu, %s -> '%s'\n",
                            fvp->flnr, col, rune_to_hex(r), ascii);
                }
                ++cnt_runes_this_line;
            }
            else {
                fputBadRune(r, dstf, fvp->flnr, col, opt);
                ++cnt_inval_this_line;
            }
        }
        ++col;
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
