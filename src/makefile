# Makfile for Infinity console game written in C++ v11
MYPROG=LooP-e
OBJS=infinityassets.o olcPixelGameEngine.o InfinityGameLogic.o infinitygame.o
HDRS=infinityassets.hpp infinityGameLogic.hpp olcPixelGameEngine.h
OUTPUTDIR=../

COMP=gcc
CFLAGS=-pipe -c -std=c++11 -MMD -MP -Wall -fpermissive -pthread -I/usr/include -I/home/steve/Documents/Shared_Max_OS/smlnd-common-inc
LINKER=g++
LFLAGS=-L/usr/lib -L/usr/lib/x86_64-linux-gnu -lGL -lX11 -lpthread -lpng -lSDL2 -lSDL2_mixer
RM=rm -f

# clean all built files
game: all
	cd $(OUTPUTDIR) && ./$(MYPROG)

# make everything
all: $(MYPROG)

# link and build executable binary
link: $(MYPROG)

$(MYPROG): compile
	$(LINKER) $(LFLAGS) $(OBJS) -o $(OUTPUTDIR)$(MYPROG)

# make everything
compile: $(OBJS)

# compile programs
%.o: %.cpp $(HDRS)
	$(COMP) $(CFLAGS) $< -o $@

# clean object files
clean:
	$(RM) $(OBJS)

# clean all backup src code files
cleansrc:
	$(RM) *~

# clean all built files
cleanall: clean cleansrc
	$(RM) $(MYPROG) $(OUTPUTDIR)$(MYPROG)
