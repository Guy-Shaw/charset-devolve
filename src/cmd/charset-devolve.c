/*
 * Filename: src/cmd/charset-devolve.c
 * Project: charset-devolve
 * Brief: Devolve richer character sets down to 7-bit ASCII
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

#include <ctype.h>
    // Import isprint()
#include <stdbool.h>
    // Import type bool
    // Import constant false
    // Import constant true
#include <stddef.h>
    // Import constant NULL
#include <stdio.h>
    // Import constant EOF
    // Import type FILE
    // Import fclose()
    // Import fopen()
    // Import fprintf()
    // Import fputc()
    // Import fputs()
    // Import getc()
    // Import putc()
    // Import snprintf()
    // Import var stdin
    // Import stdio()
    // Import var stdout
#include <stdlib.h>
    // Import abort()
    // Import exit()
#include <string.h>
    // Import strcmp()
#include <unistd.h>
    // Import getopt_long()
    // Import optarg()
    // Import opterr()
    // Import optind()
    // Import optopt()
    // Import type size_t

#define IMPORT_FVH
#include <cscript.h>
#include <utf.h>

static inline size_t
int_to_size(int i)
{
    if (i < 0) {
        abort();
    }
    return ((size_t)i);
}

enum cset {
    CHARSET_UTF8,
    CHARSET_LATIN1,
};

extern char *rune_lookup(Rune);

const char *program_path;
const char *program_name;

size_t filec;               // Count of elements in filev
char **filev;               // Non-option elements of argv

bool debug       = false;
bool verbose     = false;
bool show_counts = false;
bool show_8bit   = false;

static enum cset charset = CHARSET_UTF8;

FILE *errprint_fh = NULL;
FILE *dbgprint_fh = NULL;

static struct option long_options[] = {
    {"help",           no_argument,       0,  'h'},
    {"version",        no_argument,       0,  'V'},
    {"verbose",        no_argument,       0,  'v'},
    {"debug",          no_argument,       0,  'd'},
    {"show-counts",    no_argument,       0,  'c'},
    {"counts",         no_argument,       0,  'c'},
    {"count-8bit",     no_argument,       0,  '8'},
    {"charset",        required_argument, 0,  'C'},
    {0, 0, 0, 0}
};

static const char usage_text[] =
    "Options:\n"
    "  --help|-h|-?    Show this help message and exit\n"
    "  --version       Show version information and exit\n"
    "  --debug|-d      debug\n"
    "  --verbose|-v    verbose\n"
    "  --show-counts   After each file, show counts of devolved characters\n"
    "  --counts\n"
    "  --count-8bit    Show counts, but only if there are any non-ascii\n"
    "  --charset <charset>\n"
    "                  Specify a the source character set (encoding)\n"
    "                  Character sets are: UTF-8 latin1\n"
    "                  Default is UTF-8\n"
    "\n"
    "Only UTF-8 and latin1 are directly supported, for now.\n"
    "Other character sets could be handled by using recode\n"
    "to convert to UTF-8 or latin1, then running charset-devolve.\n"
    ;

static const char version_text[] =
    "0.1\n"
    ;

static const char copyright_text[] =
    "Copyright (C) 2016 Guy Shaw\n"
    "Written by Guy Shaw\n"
    ;

static const char license_text[] =
    "License GPLv3+: GNU GPL version 3 or later"
    " <http://gnu.org/licenses/gpl.html>.\n"
    "This is free software: you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n"
    ;

static void
fshow_program_version(FILE *f)
{
    fputs(version_text, f);
    fputc('\n', f);
    fputs(copyright_text, f);
    fputc('\n', f);
    fputs(license_text, f);
    fputc('\n', f);
}

static void
show_program_version(void)
{
    fshow_program_version(stdout);
}

static void
usage(void)
{
    eprintf("usage: %s [ <options> ]\n", program_name);
    eprint(usage_text);
}

enum variant {
    VARIANT_WORDS,
    VARIANT_ACRONYM,
};

static int
variant_strcmp(const char *var_str, const char *ref_str, int flags)
{
    const char *vp;
    const char *rp;
    const char *vw;
    const char *rw;

    (void)flags;
    vp = var_str;
    rp = ref_str;
    while (true) {
        size_t vlen;
        size_t rlen;
        size_t cmplen;
        int cmp;

        vw = vp;
        while (*vp && isalnum(*vp) && !(vp > vw && islower(vp[-1]) && isupper(*vp))) {
            ++vp;
        }
        vlen = vp - vw;
        rw = rp;
        while (*rp && isalnum(*rp) && !(rp > rw && islower(rp[-1]) && isupper(*rp))) {
            ++rp;
        }
        rlen = rp - rw;

        cmplen = vlen <= rlen ? vlen : rlen;
        cmp = strncasecmp(vw, rw, cmplen);
        if (cmp != 0) {
            return (cmp);
        }
        if (vlen < rlen) {
            return (- *rp);
        }
        else if (vlen > rlen) {
            return (*vp);
        }
        while (*vp && !isalnum(*vp)) {
            ++vp;
        }
        while (*rp && !isalnum(*rp)) {
            ++rp;
        }
        if (*vp == 0 && *rp == 0) {
            return (0);
        }
    }
}

static inline bool
is_long_option(const char *s)
{
    return (s[0] == '-' && s[1] == '-');
}

static inline char *
vischar_r(char *buf, size_t sz, int c)
{
    if (isprint(c)) {
        buf[0] = c;
        buf[1] = '\0';
    }
    else {
        snprintf(buf, sz, "\\x%02x", c);
    }
    return (buf);
}

static Rune
getRune(FILE *f)
{
    Rune r;
    int c;
    char char_buf[UTFmax + 1];
    size_t i;

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
    if (r == Runeerror) {
        fputs("*BAD*", f);
        return;
    }

    fputc('*', f);
    if ((r >= 0x80 && r <= 0xC1) || (r >= 0xF5 && r <= 0xFF) ) {
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
 * get read by GetRune() which advances as many bytes as are
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

static int
devolve_stream_utf8(fvh_t *fvp, FILE *dstf)
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
    while ((c = getc(srcf)) != EOF) {
        if (c <= 0x7F) {
            if (c == '\n') {
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
            fputc(c, dstf);
            continue;
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

            ungetc(c, srcf);
            r = getRune(srcf);
            ascii = rune_lookup(r);
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
    if (show_counts || (show_8bit && cnt_8bit != 0)) {
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

static void
fput_hex(int c, FILE *dstf)
{
    fprintf(dstf, "\\x%02x", c & 0xff);
}

#define latin1_table_base 0xa0

char *
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

static int
devolve_stream_latin1(fvh_t *fvp, FILE *dstf)
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

    if (show_counts || (show_8bit && file_count_runes != 0)) {
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

static int
devolve_stream(fvh_t *fvp, FILE *dstf)
{
    switch (charset) {
    default:
        abort();
        return (64);
    case CHARSET_UTF8:
        return (devolve_stream_utf8(fvp, dstf));
    case CHARSET_LATIN1:
        return (devolve_stream_latin1(fvp, dstf));
    }
}

static int
devolve_filev(size_t filec, char **filev, FILE *dstf)
{
    fvh_t fv;
    FILE *srcf;
    int rv;

    fv.filec = filec;
    fv.filev = filev;
    fv.glnr = 0;
    rv = 0;
    for (fv.fnr = 0; fv.fnr < filec; ++fv.fnr) {
        fv.fname = fv.filev[fv.fnr];
        if (strcmp(fv.fname, "-") == 0) {
            srcf = stdin;
        }
        else {
            srcf = fopen(fv.fname, "r");
        }
        if (srcf == NULL) {
            rv = 2;
            break;
        }
        fv.fh = srcf;
        rv = devolve_stream(&fv, dstf);
        fclose(srcf);
    }

    return (rv);
}

int
main(int argc, char **argv)
{
    extern char *optarg;
    extern int optind, opterr, optopt;
    int option_index;
    int err_count;
    int optc;
    int rv;

    set_eprint_fh();
    program_path = *argv;
    program_name = sname(program_path);
    option_index = 0;
    err_count = 0;
    opterr = 0;

    while (true) {
        int this_option_optind;

        if (err_count > 10) {
            eprintf("%s: Too many option errors.\n", program_name);
            break;
        }

        this_option_optind = optind ? optind : 1;
        optc = getopt_long(argc, argv, "+hVdvEHw:", long_options, &option_index);
        if (optc == -1) {
            break;
        }

        rv = 0;
        if (optc == '?' && optopt == '?') {
            optc = 'h';
        }

        switch (optc) {
        case 'V':
            show_program_version();
            exit(0);
            break;
        case 'h':
            fputs(usage_text, stdout);
            exit(0);
            break;
        case 'd':
            debug = true;
            set_debug_fh(NULL);
            break;
        case 'v':
            verbose = true;
            break;
        case 'c':
            show_counts = true;
            break;
        case '8':
            show_8bit = true;
            break;
        case 'C':
            if (variant_strcmp(optarg, "latin-1", VARIANT_WORDS) == 0) {
                charset = CHARSET_LATIN1;
            }
            else if (variant_strcmp(optarg, "iso-8859-1", VARIANT_WORDS) == 0) {
                charset = CHARSET_LATIN1;
            }
            else if (variant_strcmp(optarg, "utf-8", VARIANT_ACRONYM) == 0) {
                charset = CHARSET_UTF8;
            }
            else {
                eprintf("Unknown character set, '%s'\n", optarg);
                ++err_count;
            }
            break;
        case '?':
            eprint(program_name);
            eprint(": ");
            if (is_long_option(argv[this_option_optind])) {
                eprintf("unknown long option, '%s'\n",
                    argv[this_option_optind]);
            }
            else {
                char chrbuf[10];
                eprintf("unknown short option, '%s'\n",
                    vischar_r(chrbuf, sizeof (chrbuf), optopt));
            }
            ++err_count;
            break;
        default:
            eprintf("%s: INTERNAL ERROR: unknown option, '%c'\n",
                program_name, optopt);
            exit(64);
            break;
        }
    }

    if (debug)   { verbose     = true; }
    if (verbose) { show_counts = true; }

    if (optind < argc) {
        filec = (size_t) (argc - optind);
        filev = argv + optind;
    }
    else {
        filec = 0;
        filev = NULL;
    }

    if (verbose) {
        fshow_str_array(errprint_fh, filec, filev);
    }

    if (verbose && optind < argc) {
        eprintf("non-option ARGV-elements:\n");
        while (optind < argc) {
            eprintf("    %s\n", argv[optind]);
            ++optind;
        }
    }

    if (err_count != 0) {
        usage();
        exit(2);
    }

    if (filec) {
        rv = filev_probe(filec, filev);
        if (rv != 0) {
            exit(rv);
        }

        rv = devolve_filev(int_to_size(filec), filev, stdout);
    }
    else {
        char *fv_stdin = { "-" };
        rv = devolve_filev(1, &fv_stdin, stdout);
    }

    if (rv != 0) {
        exit(rv);
    }

    exit(0);
}
