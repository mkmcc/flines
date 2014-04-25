#
# 'make'        build executable file 'flines'
# 'make clean'  removes all .o and executable files
#

CC = gcc --std=c99
CFLAGS =  -W -Wall -pedantic -O3
LIBS = -lm

# define the C source files
SRCS = join_vtk.c
OBJS = $(SRCS:.c=.o)

MAIN = join_vtk.x

.PHONY: clean

all:    $(MAIN)
	@echo  build finished

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)
