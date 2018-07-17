CC              = g++
#TWAVEFORMBASE	= ../TWaveform-casa
CCFLAGS         = -g $(shell root-config --cflags)
INCLUDE         = -I$(shell root-config --incdir) -I$(TWAVEFORMBASE)/WaveBase
ROOTLIBS        = $(shell root-config --libs)
LIBDIRS         = -L$(shell root-config --libdir) -L$(TWAVEFORMBASE)/lib
TWAVEFORMLIB	= -lWaveWaveBase
binaryToROOT-NGM3316:	binaryToROOT-NGM3316.cc
		$(CC) $(CCFLAGS) $(INCLUDE) $(LIBDIRS) $(ROOTLIBS) $(TWAVEFORMLIB) \
		-o binaryToROOT-NGM3316 binaryToROOT-NGM3316.cc

clean:
		rm -f binaryToROOT-NGM3316
