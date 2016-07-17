# charset-devolve -- Devolve richer character sets to 7-bit ASCII

## 

Character sets such as Unicode, and eight-bit character sets,
like Latin1 have single code points that are not representable
as a single 7-bit ASCII character, but _are_ representable as a
short sequence of ASCII characters.

For example, the Copyright symbol can be represented,
in 7-bit ASCII without too much loss, as the 3-character sequence, "(C)".

The program, `charset-devolve` handles the most common cases
of characters used in these richer character sets.


## Options and commands

--help

--version

--debug

Pretty-print values of interest only for debugging.

--verbose

Show some feedback while running.

--latin1

The input character set is ISO/IEC 8859-1, AKA Latin1.

The default is UTF-8.

For other character sets, you can use `recode`
to convert to UTF-8 or to Latin1, then run `charset-devolve`.

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
document I encounter just have an occasional M-dash, or "real"
left and right quote marks, an apostrophe -- that's it.
No foreign language, except maybe an occasional borrowed
word, like "facade" with a real cedilla.

No need for Unicode, here.  Not really.

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

