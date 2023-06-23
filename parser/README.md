GPI FONT PARSER
===============

This is a clean implementation (based on a combination of public documentation
and trial-and-error) of a parser for the OS/2 GPI raster font format.  It ha
no dependencies on any OS/2-specific code, and should build with any ANSI C
compiler.  It requires several of the header files in the `..\include`
directory.

I originally wrote this with the hope of eventually converting it into a driver
for the FreeType library.  However, the procedure for writing FreeType drivers
is not well documented, and I never had the time to dig into the FreeType
internals to the necessary degree to figure out how to accomplish the task.  If
anyone wishes to volunteer for the job, they are more than welcome to do so. :)

The program `os2font` parses a GPI raster font from a file and displays summary
information.  It can also preview a specific character glyph from the font by
printing it to the terminal as an ASCII-simulated bitmap.  It has the ability 
to copy the font into a new file with all or a subset of its glyphs; when doing
so, you have the option of explicitly setting the saved font's target DPI to 96
or 120 (which will automatically convert the nominal point size as needed).  
Run the program with no parameters for an explanation of the syntax.

Alexander Taylor
