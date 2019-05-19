OBJS   = pacman.o monster.o util.o movie.o
CFLAGS = -W -Wall -Wextra -g -O2
LDLIBS = -lcurses

all: pacman

pacman:	$(OBJS)

pacman.o:  pacman.c  pacdefs.h
monster.o: monster.c pacdefs.h
util.o:    util.c    pacdefs.h
movie.o:   movie.c   pacdefs.h

install: pacman
	install -b -s pacman /usr/games/pacman

clean:
	-rm -f *.o

distclean: clean
	-rm -f pacman errs core a.out
