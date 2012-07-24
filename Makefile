-include config.mk

LIBNDM_MAJOR := 0
LIBNDM_MINOR := 0.0
-include version.mk

VERSION=$(LIBNDM_MAJOR).$(LIBNDM_MINOR)

ifeq ($(CC),)
	CC=cc
endif

ifeq ($(STRIP),)
	STRIP=strip
endif

.PHONY: all tests install clean distclean

STRIPFLAGS=-s -R.comment -R.note -R.eh_frame -R.eh_frame_hdr
CFLAGS?=\
	-g3 -pipe -fPIC -std=c99 \
	-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 \
	-D_POSIX_C_SOURCE=199309L \
	-ffunction-sections -fdata-sections -fstack-protector-all -Wempty-body \
	-Wall -Winit-self -Wswitch-enum -Wundef -Wunsafe-loop-optimizations \
	-Waddress -Wmissing-field-initializers -Wnormalized=nfkc -Wconversion \
	-Wredundant-decls -Wvla -Wstack-protector -ftabstop=4 -Wshadow \
	-Wpointer-arith -Wtype-limits -Wclobbered -I$(PWD)/include/
LDFLAGS=-lrt #-Wl,--gc-sections,--relax
LIB=libndm.so

PREFIX=/usr
EXEC_PREFIX=$(PREFIX)
LIB_DIR=$(EXEC_PREFIX)/lib
INCLUDE_DIR=$(PREFIX)/include

OBJS=$(patsubst %.c,%.o,$(wildcard src/*.c))
HEADERS=$(wildcard include/ndm/*.h)

TEST_DIR=tests
TEST_PREFIX=test_
TESTS=$(patsubst %.c,%,$(wildcard $(TEST_DIR)/$(TEST_PREFIX)*.c))
TEST_OBJ=$(TEST_DIR)/test.o

all: $(LIB) tests

$(LIB): Makefile $(HEADERS) $(OBJS)
	@echo LD $(LIB)
	@$(CC) $(LDFLAGS) -shared -o $@ $(OBJS)
#	$(STRIP) $(STRIPFLAGS) $@
	-@ls --block-size=K -1s $(LIB)

EXEC_TESTS=find $(TEST_DIR) -name "$(TEST_PREFIX)*" -executable -type f

check: tests
	-@for t in `$(EXEC_TESTS)`; do echo; echo "Running $$t..."; $$t; done

tests: $(LIB) $(TESTS)

install: $(LIB)
	@if [ ! -d $(EXEC_PREFIX) ]; then mkdir -p $(EXEC_PREFIX); fi
	@if [ ! -d $(INCLUDE_DIR) ]; then mkdir -p $(INCLUDE_DIR); fi
	@if [ ! -d $(LIB_DIR)     ]; then mkdir -p $(LIB_DIR); fi
	cp -r include/ndm $(INCLUDE_DIR)/ndm
	chmod 644 $(INCLUDE_DIR)/ndm/*.h
	cp $(LIB) $(LIB_DIR)
	cd $(LIB_DIR); chmod 755 $(LIB); (ldconfig || true) >/dev/null 2>&1;

$(TEST_OBJ): $(TEST_DIR)/test.c $(LIB)
	@echo "CC $<"
	@$(CC) $< $(CFLAGS) -c -o $@ >/dev/null

$(TEST_DIR)/$(TEST_PREFIX)%: $(TEST_DIR)/$(TEST_PREFIX)%.c $(TEST_OBJ) $(LIB)
	@echo "CC $<"
	@$(CC) $< $(CFLAGS) $(TEST_OBJ) $(OBJS) $(LDFLAGS) -o $@ >/dev/null

%.o: %.c ../include/ndm/%.h
	@echo "CC $<"
	@$(CC) $< $(CFLAGS) -c -o $@ >/dev/null

clean:
	rm -f src/*.o *~ *.so *.o $(LIB) $(TEST_DIR)/*.o
	rm -f `$(EXEC_TESTS)`

distclean: clean

