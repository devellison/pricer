# Pricer
Copyright (c) 2009 by Michael Ellison

This package contains my solution to a programming problem for an interview
from ages ago.

An interesting note - at the time, the x64 assembler outperformed the compilers
quite a bit - that's not really the case anymore, I'd have to rewrite it for
modern systems (if it was worth it).  The C/C++ implementations should be faster
now on any good compiler.

## Languages:

It is written in a combination of C++, C, and x86/x86-64 assembler.

C++ is used for the core of the application, as it provides the
STL containers and object-oriented support. It was obviously also
the direction the description of the programming problem pointed
in.

C is used for the configuration and external interface, as it
is generally the most easily portable and reuseable language.

Assembler is used to fix horribly slow and/or non-standard 
sprintf / itoa routines found in some C/C++ libraries.

NASM 2.07 was used as it is portable to most Intel/AMD x86 
and x86-64 platforms, which make up the majority of modern
desktop architectures.



## Source files:

   PricerConfig.h        Configuration file (overridden by Makefiles)
   Pricer.h/.cpp         C-style interface (for use as a lib/dll/etc)
   PricerParser.h        Main Parser loop.
   PricerBook.h          Order Book handler for tracking state.
   PricerOrder.h         Class to hold an individual Order's information.
   PricerStream.h/.cpp   Stream classes for unbuffered and buffered IO.
   PricerOpt.h           C-style definitions for assembler routines.
   PricerOpt.nasm        32-bit assembler itoa() replacement.
   PricerOpt64.nasm      64-bit assembler itoa() replacement.
   PricerXplat.h         Cross-platform definitions.
   PricerDefs.h          Internal C++ definitions.

## Building:


### Generic Build

unixbuild_generic.sh 

               Generates a non-assembly optimized binary on
               whatever platform.

               Output: ./bin/pricer

               Notes:
               Requires GCC/g++, but no NASM.

               I've tested it on Fedora/32, FreeBSD/32,
               cygwin, and Mac OS X Snow Leopard.

               If another build fails, then this is the 
               one to go with.  

               GCC 4.2 compiles it cleanly with -Wall.

### Mac OS X (Intel)

macbuild.sh   
 
               Generates Mac OSX 32-bit Intel and 64-bit Intel binaries.
               Output: ./bin/pricer   - 32-bit optimized Intel binary
                       ./bin/pricer64 - 64-bit optimized Intel binary.
              
               Notes:
               These are created using XCode 3.2 projects in ./projects/mac
               and have been tested under OS X Snow Leopard *only* using
               GCC 4.2.
               
               Apple's shipping version of NASM is old.  To avoid
               forcing a non-standard install, the 64-bit assembler
               is precompiled for macho64 in ./src/PricerOpt64.macho.
               
               If it fails for any reason, unixbuild_generic.sh should build
               a working build on Mac OS X, although it does not contain the
               assembler optimizations.
               
               On the bright side, at least Snow Leopard's C library has
               a pretty fast implementation of sprintf().

### Win32/64 (x86/x86-64)

winbuild.bat 

               Generates Win32 and Win64 binaries.
               Output: ./bin/pricer.exe - 32-bit optimized Intel binary.
                       ./bin/pricer64.exe - 64-bit Intel binary (no assembler)
               
               Notes: 
               These are created using VCBuild, using a Visual Studio 2008
               project in ./projects/win.  
               
               NASM 2.07 must be available in VC's binary path for the 
               32-bit versions.
               
               64-bit version has not been tested and does not contain 
               assembler optimizations.  It would require additional stub 
               code for Microsoft's 64-bit ABI in PricerOpt64.asm, although
               the core 64-bit assembler code would work.

### Linux 32/64

linux386build.sh 

               Generates i386-compatible elf32 binaries on an 
               i386 version of Linux.

               Output: ./bin/pricer

               Notes:
               This was tested with a recent i586 32-bit build
               of Fedora 11.  32-bit assembler is included,
               and NASM 2.07 must be installed and in the path.

               g++ is used for compilation.

linux64build.untested

               Generates an x86-64 compatible elf64 binary
               on an x86-64 version of Linux.

               Output: ./bin/pricer

               Notes:
               This has not been tested, as I don't currently
               have a 64-bit Linux machine running.  It will
               require NASM 2.07, and *should* work with the
               64-bit assembler optimizations.

               If it fails, the unixbuild_generic.sh script
               will build a non-optimized 64-bit version
               assuming gcc/g++ environment is default.

