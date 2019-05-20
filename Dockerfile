FROM alpine:3.9
RUN apk add --update git build-base ncurses-dev

RUN git clone --depth=1 https://github.com/troglobit/pacman.git /root/pacman
WORKDIR /root/pacman

RUN make
RUN make install DESTDIR=/tmp

FROM alpine:3.9
RUN apk add --update ncurses
COPY --from=0 /tmp/usr/games/pacman /usr/bin/

CMD [ "/usr/bin/pacman" ]
