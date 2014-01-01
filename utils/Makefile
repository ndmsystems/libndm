# Copyright (c) 2011-2013 NDM Systems, Inc. http://www.ndmsystems.com/

.PHONY: all install clean distclean

ifeq ($(CC),)
    CC=cc
endif

CONVGEN=convgen
EXEC_DIR=/usr/sbin
OBJECTS=convgen.o

CFLAGS?=\
	-g3 -pipe -fPIC -std=c99 \
	-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 \
	-D_POSIX_C_SOURCE=200809L \
	-ffunction-sections -fdata-sections -fstack-protector-all -Wempty-body \
	-Wall -Winit-self -Wswitch-enum -Wundef -Wunsafe-loop-optimizations \
	-Waddress -Wmissing-field-initializers -Wnormalized=nfkc -Wconversion \
	-Wredundant-decls -Wvla -Wstack-protector -ftabstop=4 -Wshadow \
	-Wpointer-arith -Wtype-limits -Wclobbered
LDFLAGS+=

all: $(CONVGEN)

$(CONVGEN): $(OBJECTS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

install: $(CONVGEN)
	install -D $(CONVGEN) $(EXEC_DIR)

clean:
	rm -f *.o $(CONVGEN)

distclean: clean

