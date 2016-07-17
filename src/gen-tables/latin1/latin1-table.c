/*
 * Translate table for Latin1 characters in the range 0xa0 .. 0xff.
 *
 * Most translations are to a single character.  The primary translation
 * table handles those directly.  Any entry in the range 0x00..0x7f
 * is the translation itself, but entries with the top bit set are
 * interpreted as an index into an "overflow" table for translations
 * to strings 2 characters or longer.  For example, the Latin1 Copyright
 * symbol devolves to the 3 character string, "(C)".
 *
 */

char latin1_table[] = {
    0x80,  // 0xa0
    0,  // 0xa1
    0x81,  // 0xa2
    0x82,  // 0xa3
    0x83,  // 0xa4
    0x84,  // 0xa5
    '|',  // 0xa6
    0x85,  // 0xa7
    0x86,  // 0xa8
    0x87,  // 0xa9
    0,  // 0xaa
    0x88,  // 0xab
    0,  // 0xac
    0,  // 0xad
    0x89,  // 0xae
    0,  // 0xaf
    0x8a,  // 0xb0
    0x8b,  // 0xb1
    '2',  // 0xb2
    '3',  // 0xb3
    '\'',  // 0xb4
    'u',  // 0xb5
    0x8c,  // 0xb6
    '.',  // 0xb7
    0,  // 0xb8
    '1',  // 0xb9
    0,  // 0xba
    0x8d,  // 0xbb
    0x8e,  // 0xbc
    0x8f,  // 0xbd
    0x90,  // 0xbe
    0,  // 0xbf
    'A',  // 0xc0
    'A',  // 0xc1
    'A',  // 0xc2
    'A',  // 0xc3
    'A',  // 0xc4
    'A',  // 0xc5
    0x91,  // 0xc6
    'C',  // 0xc7
    'E',  // 0xc8
    'E',  // 0xc9
    'E',  // 0xca
    'E',  // 0xcb
    'I',  // 0xcc
    'I',  // 0xcd
    'I',  // 0xce
    'I',  // 0xcf
    'D',  // 0xd0
    'N',  // 0xd1
    'O',  // 0xd2
    'O',  // 0xd3
    'O',  // 0xd4
    'O',  // 0xd5
    'O',  // 0xd6
    'x',  // 0xd7
    'O',  // 0xd8
    'U',  // 0xd9
    'U',  // 0xda
    'U',  // 0xdb
    'U',  // 0xdc
    'Y',  // 0xdd
    'P',  // 0xde
    'B',  // 0xdf
    'a',  // 0xe0
    'a',  // 0xe1
    'a',  // 0xe2
    'a',  // 0xe3
    'a',  // 0xe4
    'a',  // 0xe5
    0x92,  // 0xe6
    'c',  // 0xe7
    'e',  // 0xe8
    'e',  // 0xe9
    'e',  // 0xea
    'e',  // 0xeb
    'i',  // 0xec
    'i',  // 0xed
    'i',  // 0xee
    'i',  // 0xef
    'o',  // 0xf0
    'n',  // 0xf1
    'o',  // 0xf2
    'o',  // 0xf3
    'o',  // 0xf4
    'o',  // 0xf5
    'o',  // 0xf6
    '/',  // 0xf7
    'o',  // 0xf8
    'u',  // 0xf9
    'u',  // 0xfa
    'u',  // 0xfb
    'u',  // 0xfc
    'y',  // 0xfd
    'p',  // 0xfe
    'y',  // 0xff
};

char *zstr_table[] = {
    "\\[nbsp]",
    "\\[cents]",
    "\\[GBP]",
    "\\[lozenge]",
    "\\[Yen]",
    "\\[section]",
    "\\[umlaut]",
    "(C)",
    "<<",
    "(R)",
    "\\[degree]",
    "\\[+-]",
    "\\[paragraph]",
    ">>",
    "1/4",
    "1/2",
    "3/4",
    "AE",
    "ae",
};
