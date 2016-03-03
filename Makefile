CC              = g++
CCFLAGS         = -g $(shell root-config --cflags)
INCLUDE         = -I$(shell root-config --incdir)
ROOTLIBS        = $(shell root-config --libs)
LIBDIRS         = -L$(shell root-config --libdir)

binaryToROOT-NGM3316:	binaryToROOT-NGM3316.cc
		$(CC) $(CCFLAGS) $(INCLUDE) $(LIBDIRS) $(ROOTLIBS) \
		-o binaryToROOT-NGM3316 binaryToROOT-NGM3316.cc

clean:
		rm -f binaryToROOT-NGM3316
