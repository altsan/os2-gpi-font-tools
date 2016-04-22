This is a clean implementation (based on public documentation) of a parser for the OS/2 GPI bitmap font format.  It has no dependencies on any OS/2-specific code, and should build with any ANSI C compiler.  It does use several of the header files in the ..\include directory.

I originally wrote this with the hope of eventually converting it into a driver for the FreeType library.  However, the procedure for writing FreeType drivers is not well documented, and I never had the time to dig into the FreeType internals to the necessary degree to figure out how to accomplish the task.  If anyone wishes to volunteer for the job, they are more than welcome to do so. :)

Alexander Taylor, April 2016