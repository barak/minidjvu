CFLAGS:=-pipe -g -I.. \
        -Wall -Werror -Wshadow -pedantic-errors \
       -Wpointer-arith -Waggregate-return \
       -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations \
       -Wlong-long -Winline -Wredundant-decls -Wcast-qual -Wcast-align \
       -D__STRICT_ANSI__


../bin/minidjvu: minidjvu.c Makefile
	mkdir -p $(dir $@)
#	libtool gcc $(CFLAGS) -L../bin/ -lminidjvu $< -o $@
	gcc $(CFLAGS) $(PWD)/../bin/libminidjvu.so $< -o $@
