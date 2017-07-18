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
    // Import isalnum()
    // Import islower()
    // Import isprint()
    // Import isupper()
#include <stdbool.h>
    // Import type bool
    // Import constant false
    // Import constant true
#include <stddef.h>
    // Import constant NULL
#include <stdio.h>
    // Import type FILE
    // Import fclose()
    // Import fopen()
    // Import fputc()
    // Import fputs()
    // Import snprintf()
    // Import var stdin
    // Import var stdout
#include <stdlib.h>
    // Import abort()
    // Import exit()
#include <string.h>
    // Import strcmp()
#include <strings.h>
    // Import strncasecmp()
#include <unistd.h>
    // Import getopt_long()
    // Import type size_t

#define IMPORT_FVH
#include <cscript.h>
#include <utf.h>

#include <devolve.h>

static inline size_t
int_to_size(int i)
{
    if (i < 0) {
        abort();
    }
    return ((size_t)i);
}

const char *program_path;
const char *program_name;

size_t filec;               // Count of elements in filev
char **filev;               // Non-option elements of argv

bool debug        = false;
bool verbose      = false;

static unsigned int devolve_options = 0;
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
    {"soft-hyphens",   no_argument,       0,  'H'},
    {0, 0, 0, 0}
};

static const char usage_text[] =
    "Options:\n"
    "  --help|-h|-?    Show this help message and exit\n"
    "  --version       Show version information and exit\n"
    "  --debug|-d      debug\n"
    "  --verbose|-v    verbose\n"
    "  --soft-hyphens  Show soft hyphen as hyphen\n"
    "                  defualt is strip soft hyphens\n"
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
    "Copyright (C) 2016-2017 Guy Shaw\n"
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

/*
 * Like strcmp(), except that several variations of case and punctuation
 * are allowed.  For example, all the following compare equal:
 *
 *   my-option  my_option  MyOption MY-OPTION My-Option ...
 *
 * but not
 *
 *   my-option == myoption
 *
 * because 'my-option' is two "words" but 'myoption' is just one word.
 *
 * Word boundaries are either punctuation ('-' or '_')
 * or a CamelCase transition from lowercase to uppercase.
 *
 * Status: Experimental.  Needs work.
 *
 */

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

static int
devolve_stream(fvh_t *fvp, FILE *dstf)
{
    switch (charset) {
    default:
        abort();
        return (64);
    case CHARSET_UTF8:
        return (devolve_stream_utf8(fvp, dstf, devolve_options));
    case CHARSET_LATIN1:
        return (devolve_stream_latin1(fvp, dstf, devolve_options));
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
        case 'H':
            devolve_options |= (unsigned int)OPT_SOFT_HYPHENS;
            break;
        case 'c':
            devolve_options |= (unsigned int)OPT_SHOW_COUNTS;
            break;
        case '8':
            devolve_options |= (unsigned int)OPT_SHOW_8BIT;
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

    if (debug) {
        verbose = true;
    }
    if (verbose) {
        devolve_options |= (unsigned int)OPT_SHOW_COUNTS;
    }

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
