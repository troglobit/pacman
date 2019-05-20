OBJS   = pacman.o monster.o util.o movie.o
CFLAGS = -W -Wall -Wextra -g -O2
LDLIBS = -lcurses

all: pacman

pacman:	$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDLIBS)

pacman.o:  pacman.c  pacdefs.h
monster.o: monster.c pacdefs.h
util.o:    util.c    pacdefs.h
movie.o:   movie.c   pacdefs.h

install: pacman
	@touch /tmp/pacman.log
	install -b -D -s pacman $(DESTDIR)/usr/games/pacman
	install -b -D pacman.6 $(DESTDIR)/usr/man/man6/pacman.6
	install -b -D README.md $(DESTDIR)/usr/share/doc/pacman/README.md
	install -b -D /tmp/pacman.log $(DESTDIR)/var/games/pacman.log
	@rm /tmp/pacman.log

clean:
	-rm -f *.o

distclean: clean
	-rm -f pacman errs core a.out
