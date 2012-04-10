This is a simple C program implementing the [SexpCode](http://cairnarvon.rotahall.org/misc/sexpcode.html) standard. It reads SexpCode code from standard input, converts it to HTML, and writes that HTML to standard output.

It implements the entire standard, plus a non-standard `ruby` function, which maps to the HTML 5 [`<ruby>` tag](http://www.w3schools.com/html5/tag_ruby.asp).

## System requirements

This code assumes a POSIX system and a C89 compiler, but nothing else.

## Installation instructions

If you have `make` installed, run `make`. `make install` if you want it to move the resulting executable to your `~/bin` folder. (This will only be useful if `~/bin` is in your `$PATH`.)

Otherwise compile it as normal and put it wherever.

## Usage

>     $ echo "{b test}" | sexpcode
>     <b>test</b>

SexpCode is translated to output in three passes: one to deal with backslash escapes and the alternative verbatim syntax, one to translate the result to normal form (i.e. without composed or iterated functions or partial application, and with definitions substituted), and one to turn normal-form SexpCode into the output. If you want to stop after the first or second of these passes, the program recognises the command line arguments `-pass0` and `-pass1`, respectively.

To use this parser as a library, `sexpcode.h` exposes the relevant things, and all that is required of you is that you set a `get_func` to be used. The program (`main.c`) sets this to the `html_get_func` exposed by `html.h`, but you could write your own; it should take a `char*` holding a function name, and return either a pointer to `sexpcode_func` (see `sexpcode.h`) or `NULL` in case of an invalid function name.

If you would like to implement your own output functions, look at `html.h` and `html.c` for inspiration. By way of example, `term.h` and `term.c` implement a small subset of functions for use on an ANSI terminal (emulator).

If you just want to tweak the HTML output, look at `html.c`.

## Bugs

Report bugs here or in `#rotahall` on irc://irc.synirc.net.
