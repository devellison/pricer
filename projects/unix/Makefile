#!/bin/sh

CPP      = g++
CPPFLAGS = -c -O3 -Wall

srcdir = ../../src
bindir = ../../bin
objdir = ../../obj

cppobjects = Pricer.o       \
             PricerMain.o   \
             PricerStream.o

headers = $(srcdir)/Pricer.h          \
          $(srcdir)/PricerBook.h      \
          $(srcdir)/PricerConfig.h    \
          $(srcdir)/PricerDefs.h      \
          $(srcdir)/PricerOrder.h     \
          $(srcdir)/PricerParser.h    \
          $(srcdir)/PricerStream.h    \
          $(srcdir)/PricerXplat.h     \
          $(srcdir)/PricerOpt.h

Default: pricer

pricer: objdirmk $(cppobjects)
	$(CPP) -o $(bindir)/pricer $(objdir)/*.o

Pricer.o: $(srcdir)/Pricer.cpp $(headers) objdirmk
	$(CPP) $(CPPFLAGS) $(srcdir)/Pricer.cpp -o $(objdir)/Pricer.o

PricerMain.o: $(srcdir)/PricerMain.cpp $(headers) objdirmk
	$(CPP) $(CPPFLAGS) $(srcdir)/PricerMain.cpp -o $(objdir)/PricerMain.o

PricerStream.o: $(srcdir)/PricerStream.cpp $(headers) objdirmk
	$(CPP) $(CPPFLAGS) $(srcdir)/PricerStream.cpp -o $(objdir)/PricerStream.o

objdirmk:
	rm -Rf $(objdir)
	mkdir -p $(objdir)

clean: 
	rm -Rf $(objdir)
	rm $(bindir)/pricer

