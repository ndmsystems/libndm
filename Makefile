-include config.mk

LIBNDM_MAJOR := 0
LIBNDM_MINOR := 0.0
-include version.mk

UNAME  =$(shell uname)
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
	-D_POSIX_C_SOURCE=200809L -D_BSD_SOURCE \
	-ffunction-sections -fdata-sections -fstack-protector-all \
	-Wall -Winit-self -Wswitch-enum -Wundef \
	-Wmissing-field-initializers -Wconversion \
	-Wredundant-decls -Wstack-protector -ftabstop=4 -Wshadow \
	-Wpointer-arith -I$(PWD)/include/
#	-Wempty-body -Wclobbered -Waddress -Wvla -Wtype-limits
LDFLAGS=-lc #-Wl,--gc-sections,--relax
ifneq ($(UNAME),Darwin)
LDFLAGS+=-lrt
endif
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
TEST_NO_AUTOEXEC=core core_event

EXAMPLE_DIR=examples
EXAMPLES=$(patsubst %.c,%,$(wildcard $(EXAMPLE_DIR)/*.c))

ifeq ($(filter memory_debug,$(MAKECMDGOALS)),memory_debug)
MEM_DEBUG_OBJ:=$(TEST_DIR)/memchk.o
MEM_DEBUG_DEFS:=-D_MEMORY_LEAK_DEBUG -D_MEMORY_OVERFLOW_DEBUG -pthread
MEM_DEBUG_CFLAGS:=$(CFLAGS) $(MEM_DEBUG_DEFS)

CFLAGS+=$(MEM_DEBUG_DEFS) -include tests/memchk.h
OBJS+=$(MEM_DEBUG_OBJ)

$(MEM_DEBUG_OBJ): $(TEST_DIR)/memchk.c
	@echo "CC $<"
	@$(CC) $< $(MEM_DEBUG_CFLAGS) -c -o $@ >/dev/null
endif

all: $(LIB)

$(LIB): Makefile $(HEADERS) $(OBJS)
	@echo LD $(LIB)
	@$(CC) -shared -o $@ $(OBJS) $(LDFLAGS)
#	$(STRIP) $(STRIPFLAGS) $@
	-@ls -k -1s $(LIB)

EXEC_TESTS_ALL=\
	$(shell find $(TEST_DIR) -name "$(TEST_PREFIX)*" -perm -u=x -type f)
EXEC_TESTS=$(filter-out $(addprefix \
	$(TEST_DIR)/$(TEST_PREFIX),$(TEST_NO_AUTOEXEC)),$(EXEC_TESTS_ALL))
EXEC_EXAMPLES_ALL=$(shell find $(EXAMPLE_DIR) -perm -u=x -type f)

check: tests
	-@for t in $(EXEC_TESTS); do echo; echo "Running $$t..."; $$t; done

tests: $(LIB) $(TESTS)

examples: $(LIB) $(EXAMPLES)

memory_debug: check

install: $(LIB)
	@if [ ! -d $(EXEC_PREFIX) ]; then mkdir -p $(EXEC_PREFIX); fi
	@if [ ! -d $(INCLUDE_DIR) ]; then mkdir -p $(INCLUDE_DIR); fi
	@if [ ! -d $(LIB_DIR)     ]; then mkdir -p $(LIB_DIR); fi
	cp -r include/* $(INCLUDE_DIR)
	chmod 755 $(INCLUDE_DIR)/ndm
	chmod 644 $(INCLUDE_DIR)/ndm/*.h
	cp $(LIB) $(LIB_DIR)
	cd $(LIB_DIR); chmod 755 $(LIB); (ldconfig || true) >/dev/null 2>&1;

$(TEST_OBJ): $(TEST_DIR)/test.c $(LIB)
	@echo "CC $<"
	@$(CC) $< $(CFLAGS) -c -o $@ >/dev/null

$(TEST_DIR)/$(TEST_PREFIX)%: $(TEST_DIR)/$(TEST_PREFIX)%.c $(TEST_OBJ) $(LIB)
	@echo "CC $<"
	@$(CC) $< $(CFLAGS) $(TEST_OBJ) $(OBJS) $(LDFLAGS) -o $@ >/dev/null

$(EXAMPLE_DIR)/%: $(EXAMPLE_DIR)/%.c $(LIB)
	@echo "CC $<"
	@$(CC) $< $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@ >/dev/null

%.o: %.c ../include/ndm/%.h
	@echo "CC $<"
	@$(CC) $< $(CFLAGS) -c -o $@ >/dev/null

clean:
	rm -f src/*.o *~ *.so *.o $(LIB) $(TEST_DIR)/*.o $(EXAMPLE_DIR)/*.o
	rm -f $(EXEC_TESTS_ALL)
	rm -f $(EXEC_EXAMPLES_ALL)

distclean: clean

