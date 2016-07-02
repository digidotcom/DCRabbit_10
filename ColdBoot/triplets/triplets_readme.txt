Author: Bryce Hathaway
Date: December 10, 2007

This document discusses the following files:
triplets.bpr, triplets.cpp, triplets.exe


Command line synopsis:
triplets.exe <input_file> <output_file>

Looks for "input_file.c" and "input_file.bin" and
creates "output_file.bin".
Assumes input_file != output_file.


Note: 
Even though this utility simplifies cold loader
development, do not modify cold loader source files
unless you know exactly what you are doing!

Info:
The utility "triplets.exe" takes a cold loader source
file and binary and generates the triplets necessary to
boot a Rabbit processor. Unlike its predecessor, 
"makecold.cpp", one does not compile this utility for
each cold loader build because it expects the cold
loader source file to specify any additional I/O settings as special comments. Specifically, the utility
searches for lines containing "//@", and interprets what follows as a command. As currently implemented, the commands are either an I/O register followed by a value for that register, or the special "start" command which specifies the starting address of the cold loader. All I/O setting, regardless of their placement
in relation to source code, the utility sequentially
processes by finding the I/O address for the register,
setting the high bit of the 16-bit address, and
affixing the data byte to the end. For example,
//@ MB0CR 0xC6
is transated to the following triplet:
0x80 0x14 0xC6
in increasing address order left-to-right.

In boot strap mode, the Rabbit cannot write beyond
8-bit I/O addresses, so although the utility can
understand 15-bit I/O registers, one should not use them. The author implemented this behavior in case
future Rabbit processors lift this limitation. Also, do
not specify 16-bit values for I/O registers, since boot
strap mode does not support this.

Either the end of the source file or the "start"
command signal the end of the I/O prefix triplet
generation. The utility will then generate the I/O
triplets and proceed to the binary file. The utiliy
begins with the address it found in the source file
or zero if it found the end of file first. It reads the
bytes of the binary file and prefixes them with
increasing addresses. For example, if the beginning of
the binary file looked like this:
0x3E 0xC0 0xD3 ...
the utility would generate:
0x10 0x00 0x3E  0x10 0x01 0xC0  0x10 0x02 0xD3
if the "start" address had been 0x1000.

Finally the utility stops generating triplets after it
finds the special byte sequence 0x76 0x76 0x76 0x76 or
the end of file. It then appends 0x80 0x24 0x80, the
special I/O command that signals the Rabbit processor
to run from address zero.


Additional Notes:
If this utility has bugs or if one needs to modify it,
open triplets.bpr using Borland C++ builder, which
should have triplets.cpp as a source file for the
command line utility executable file.