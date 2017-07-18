/*
 * Filename: src/inc/devolve.h
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

#ifndef _DEVOLVE_H
#define _DEVOLVE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>
    // Import type FILE

/*
 * List of character sets we know how to handle.
 */

enum cset {
    CHARSET_UTF8,
    CHARSET_LATIN1,
};

/*
 * Options.
 * A main() program parses options,
 * then passes those encoded options libdevolve functions.
 */

enum devolve_option {
    OPT_SOFT_HYPHENS = 0x01,
    OPT_SHOW_COUNTS  = 0x02,
    OPT_SHOW_8BIT    = 0x04,
};

#if 0

#define OPT_SOFT_HYPHENS 0x01
#define OPT_SHOW_COUNTS  0x02
#define OPT_SHOW_8BIT    0x04

#endif

extern int  devolve_stream_utf8(fvh_t *fvp, FILE *dstf, unsigned int opt);
extern int  devolve_stream_latin1(fvh_t *fvp, FILE *dstf, unsigned int opt);

#ifdef  __cplusplus
}
#endif

#endif  /* _DEVOLVE_H */
