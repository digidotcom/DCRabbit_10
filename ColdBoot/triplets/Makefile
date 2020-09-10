# Make file written June 2009 by Tom Collins

# use -static so files compiled in MSYS2/MinGW64 work from cmd prompt
CPPFLAGS=-O2 -Wall -static

all: triplets.exe BL_triplets.exe

triplets.exe: triplets.cpp
	g++ ${CPPFLAGS} -o triplets.exe triplets.cpp

BL_triplets.exe: BL_triplets.cpp
	g++ ${CPPFLAGS} -o BL_triplets.exe BL_triplets.cpp
