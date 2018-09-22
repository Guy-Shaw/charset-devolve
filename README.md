# Devolve richer character sets to 7-bit ASCII

## 

Character sets such as Unicode, and eight-bit character sets,
like Latin1 have single code points that are not representable
as a single 7-bit ASCII character, but _are_ representable as a
short sequence of ASCII characters.

For example, the Copyright symbol can be represented,
in 7-bit ASCII without too much loss, as the 3-character sequence,

   "(C)".

The program, `charset-devolve`, handles the most common cases
of characters used in these richer character sets.


## Options and commands

`--help`

`--version`

`--debug`

Pretty-print values of interest only for debugging.

`--verbose`

Show some feedback while running.

`--charset=utf8`

The input character set is UTF-8.

`--charset=latin1`

The input character set is ISO/IEC 8859-1, AKA Latin1.

The default is UTF-8.

For other character sets, you can use `recode`
to convert to UTF-8 or to Latin1, then run `charset-devolve`.

`--show-counts`

At the end, show counts of errors, bytes that are invalid UTF-8,
bytes with high bit set (whether UTF-8 or not), number of UTF-8 runes,
number of lines containing errors, 8-bit characters, UTF8 runes.

`--count-8bit`

Like option, `--show-counts`, except that it only shows counts
if there are any bytes with the hight bit turned on.
Nothing would be reported if the input were pure 7-bit ASCII.

`--soft-hyphens`

Normally, Unicode SOFT HYPHEN (U+00AD) is supressed.
But, there are times when you might want to see them.
If the option, '--soft-hyphens' is specified,
then soft hyphens get devolved into ASCII dash/minus/hyphen,
0x2D.  This option applies only to --charset=utf8.

`--trace-conv`

Trace conversions on stderr as they happen.

If there are many conversions, then the trace can be pretty
noisy, but often there are only a few actual modifications
even in a large text file.  In that case, it can be useful
to see the report of just the characters that have been modified.

`--trace-errors`

Trace invalid UTF-8 byte sequences on stderr.

`--trace-untrans`

Trace Non-ASCII UTF-8 runes that are valid,
but have no translation (cannot be devolved).
Trace on stderr.


## Exit Status

If there were no invalid UTF-8 runes in the input,
then `charset-devolve` returns exit status = 0.
If there, were any bytes that are not valid UTF-8,
then exit status is 1.  More serious errors,
such as failure to open a file, cause exit status 2, or greater.


## Why

Although, I believe Unicode is the way of the future,
and I am sympathetic to the position of "Unicode Damnit"
and other proponents of converting everything to Unicode,
I have situations that come up in the real world, today,
for which conversion to Unicode is impractical or overkill.

I deal with text files that are composed by cut-and-paste,
from various sources, with various encodings.  None of these
sources involve any serious use of richer character sets.
They are almost all English language.  Many UTF-8 encoded
documents I encounter just have an occasional M-dash, or "real"
left and right quote marks, an apostrophe,
a Copyright or Trademark symbol -- that's it.

No foreign language, except maybe an occasional borrowed
word, like "facade" with a real cedilla.

No need for Unicode, here.  Not really.

## Other Uses

Because `charset-devolve` can tell if there are _any_
bytes that are not valid UTF-8, and can return a reliable
exit status, I have found that is a more reliable test
(of UTF8 or not-UTF8) than some of the programs
that try to guess the character encoding of a given file.

Where I used to try a three-step process:

  1. get encoding from metadata;
  2. failing that, guess encoding;
  3. convert to UTF8, if not already UTF8.

Now, I run `charset-devolve --charset=utf8`.
Most of the time (in my experience, your mileage may vary),
it works.  But, if it reports that the file contains some
non-UTF8 content, then, fall back on the programs that guess
character set.

## License

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

##

-- Guy Shaw

   gshaw@acm.org

