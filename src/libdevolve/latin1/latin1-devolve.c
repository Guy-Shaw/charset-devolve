#include <cscript.h>
#include <stdlib.h>	// Import EXIT_SUCCESS
#include <ctype.h>	// Import isprint()

typedef size_t index_t;
typedef unsigned int uint_t;

const char *program_path;
const char *program_name;

FILE *errprint_fh;
FILE *dbgprint_fh;

bool verbose;
bool debug;

static struct option long_options[] = {
    {"verbose", no_argument,       0,  'v'},
    {"debug",   no_argument,       0,  'd'},
    {0,         0,                 0,  0  }
};

static const char usage_text[] =
    "Options:\n"
    "  -v|verbose         verbose\n"
    "  -d|debug           debug\n";

static void
usage()
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

static void
latin1_devolve_stream(FILE *srcf, FILE *dstf)
{
    int c;

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
}

int
main(int argc, char **argv)
{
    extern char *optarg;
    extern int optind, opterr, optopt;
    int option_index;
    int err_count;
    int optc;

    program_path = *argv;
    program_name = sname(program_path);
    option_index = 0;
    err_count = 0;
    opterr = 0;

    set_eprint_fh();
    while (true) {
        int this_option_optind;

        if (err_count > 10) {
            eprintf("%s: Too many options errors.\n", program_name);
            break;
        }

        this_option_optind = optind ? optind : 1;
        optc = getopt_long(argc, argv, "dv", long_options, &option_index);
        if (optc == -1) {
            break;
        }

        switch (optc) {
        case 'd':
            debug = true;
            set_debug_fh("");
            break;
        case 'v':
            verbose = true;
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
    latin1_devolve_stream(stdin, stdout);
    exit(EXIT_SUCCESS);
    return (EXIT_SUCCESS);
}
