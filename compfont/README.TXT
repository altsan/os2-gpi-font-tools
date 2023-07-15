COMPFONT - OS/2 Composite Font Editor/Inspector
(C) 2016-2023 Alexander Taylor

This is intended to be a simple graphical tool for creating, editing, or
examining the various composite font formats defined by OS/2 Presentation
Manager.

This program is still under development, and does not yet support modifying
or creating files.  At present, it may be used to examine existing composite
font files, with some limitations.

The term 'composite font' is used by this program as an overall term for
several different (but related) font formats, described below.  Eventually,
COMPFONT should be capable of opening and parsing all of these formats.

Of the types listed below, alias files and associated bitmap rules are known
to be implemented in OS/2 4.5x.  It is uncertain whether the other types are
supported or not.


COMBINED FONT FILE

An OS/2 'combined font' is a special file which allows different fonts (or
subsets thereof) to be logically combined into one larger font.  To quote
from the IBM document which defines the format:

   Developing Unicode fonts, that must include a huge number of characters,
   usually takes a large amount of time and money.

   One idea to resolve this problem is to provide a solution to create
   logically described fonts.  The font will look like other regular fonts
   from user's point of view and it can have the Unicode encoding.  The
   font does not have an actual font file but is virtually created by the
   system by putting some of the existing fonts together into one logically
   defined font. We will call the logical font a combined font since the
   font is created by combining existing fonts.
                                  -- IBM, 'CmbFont' document, December 1997

Besides the function described above, there are several specialized variants
of the combined font format used for particular purposes:

  Alias File:  Essentially a combined font with only one component.  This is
      used to provide an alias which maps one font name to another, generally
      for compatibility purposes.  Since the release of WSeB, OS/2 has
      included one such font out of the box: TNRMT30.CMB, which aliases the
      name "Times New Roman MT 30" to the Unicode TrueType font "Times New
      Roman WT J".

  Associated Bitmaps Rule File:  Used in DBCS versions of OS/2 in order to
      allow CJK ideographic characters in the included TrueType fonts to be
      substituted with bitmap glyphs at select resolutions.  This is done
      because complicated TrueType glyphs can easily become illegible when
      rendered on-screen at low-to-moderate resolutions, whereas hand-tuned
      bitmaps are much more readable.  (This technique pre-dates the
      widespread availability of TrueType fonts with embedded bitmaps, which
      serve a similar purpose.)

  Pre-Combine Rule File:  Unlike a regular combined font, a PCR does not
      create a separate logical font with a new name.  Instead, it allows an
      existing font to load characters from another, associated, font on
      demand whenever an application requests a character which the original
      font does not support.  (NOTE: This format was defined in IBM's design
      documentation, but it is unclear whether or not it was ever actually
      implemented.)


UNI-FONT FILE

A Uni-font file is a special format defined by OS/2 Presentation Manager.
Broadly speaking, it is an extension of the OS/2 bitmap font format that
supports native Unicode mapping and very large character sets.  It achieves
this by combining multiple bitmap font resources in a single file, with a
header format similar to that of a combined font file.  (IBM's notes on the
combined font format suggest that its design was based on that of the Uni-font
format.)

There are important differences between the Uni-font format and the combined
font format.  In particular, all of the components are included in the same
Uni-font file itself - no mapping to external files is performed.  Also, the
font resources themselves all use the same bitmap format; outline glyph data
is not supported.

The author is not actually aware of any extant Uni-font files, or indeed
whether or not support for this format was ever fully implemented.  The name
of the corresponding OS/2 font driver, PMUNIF.DLL, suggests that the Uni-font
format is supported in theory; however, this driver apparently also implements
support for the combined font format, and is probably included in OS/2 for
this reason.

Some logic for reading this format has been implemented in COMPFONT; however
it is incomplete and (lacking any actual fonts to test with) 100% untested.


LICENSE

OS/2-GPI Composite Font Editor/Inspector
Copyright (C) 2016-2023 Alexander Taylor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
