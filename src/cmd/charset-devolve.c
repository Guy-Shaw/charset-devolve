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

bool verbose = false;
bool debug   = false;

static enum cset charset = CHARSET_UTF8;

FILE *errprint_fh = NULL;
FILE *dbgprint_fh = NULL;

static struct option long_options[] = {
    {"help",           no_argument,       0,  'h'},
    {"version",        no_argument,       0,  'V'},
    {"verbose",        no_argument,       0,  'v'},
    {"debug",          no_argument,       0,  'd'},
    {"charset",        required_argument, 0,  'C'},
    {0, 0, 0, 0}
};

static const char usage_text[] =
    "Options:\n"
    "  --help|-h|-?         Show this help message and exit\n"
    "  --version            Show version information and exit\n"
    "  --verbose|-v         verbose\n"
    "  --debug|-d           debug\n"
    "  --charset <charset>  Use a character encoding other than UTF-8\n"
    "\n"
    "Only latin1 is directly support, for now.\n"
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

static int
devolve_stream_utf8(fvh_t *fvp, FILE *dstf)
{
    FILE *srcf;
    Rune r;

    srcf = fvp->fh;
    fvp->flnr = 0;
    while ((r = getRune(srcf)) != (Rune) EOF) {
        char *ascii;

        if (r <= 0x7F) {
            if (r == '\n') {
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
        }
    }

    return (0);
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
    FILE *srcf;
    int c;

    srcf = fvp->fh;
    fvp->flnr = 0;
    while ((c = getc(srcf)) != EOF) {
        char *ascii;

        if (c <= 0x7F) {
            putc(c, dstf);
            continue;
        }
        ascii = latin1_devolve_chr(c);
        if (ascii != NULL) {
            fputs(ascii, dstf);
        }
        else {
            fput_hex(c, dstf);
        }
    }

    return (0);
}

static int
devolve_stream(fvh_t *fvp, FILE *dstf)
{
    switch (charset) {
    default:
        abort();
        return (-1);
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
        devolve_stream(&fv, dstf);
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
        case 'C':
            charset = CHARSET_LATIN1;
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
            exit(2);
            break;
        }
    }

    verbose = verbose || debug;

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
        exit(1);
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
