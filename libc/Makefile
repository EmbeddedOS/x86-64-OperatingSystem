
SC=gcc
SCC=g++
FLAGS=-w -g -ffreestanding  -I ./include -c

OBJS= ./string.c     \
      ./strings.c    \
      ./list.c


OBJO = $(OBJS:.c=.o) 
OBJ=libc.a

all: $(OBJ)
	@echo "Building libc."

libc.a: $(OBJO)
	ar rcs $@ *.o

%.o: %.c
	$(SC) $(FLAGS) $<

clean:
	rm -f $(OBJ) *.o *.a