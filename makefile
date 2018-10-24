#
MYPROG=hello
OBC_OBJS=main.o
CPP_OBJS=myobj.o
OBC_SRC=main.mm
CPP_SRC=myobj.cpp
OBC_HDRS=
CPP_HDRS=myobj.hpp
OBC_COMP=gcc
CPP_COMP=gcc
OBC_CFLAGS=-c `gnustep-config --objc-flags` -I/usr/include/GNUstep
CPP_CFLAGS=-c -I/usr/include
LINKER=g++
LFLAGS=-lobjc -L/usr/lib -L/usr/lib/x86_64-linux-gnu -lgnustep-base -fconstant-string-class=NSConstantString
OUTPUTDIR=./
RM=rm -f *.o *.d *~ hello main *.out

# run program
run: all
	cd $(OUTPUTDIR) && ./$(MYPROG)

#make all
all: $(MYPROG)

# link program
$(MYPROG): compile
	$(LINKER) $(LFLAGS) $(OBC_OBJS) $(CPP_OBJS) -o $(OUTPUTDIR)$(MYPROG)

# make everything
compile: compile_obc compile_cpp

compile_obc: $(OBC_OBJS)

compile_cpp: $(CPP_OBJS)

# compile programs
%.o: %.m $(OBC_HDRS)
	$(OBC_COMP) $(OBC_CFLAGS) $< -o $@

# compile programs
%.o: %.mm $(OBC_HDRS)
	$(OBC_COMP) $(OBC_CFLAGS) $< -o $@

# compile programs
%.o: %.cpp $(CPP_HDRS)
	$(CPP_COMP) $(CPP_CFLAGS) $< -o $@

# clean files
clean:
	$(RM)
