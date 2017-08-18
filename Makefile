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

.PHONY: all static sanitize memory_debug tests install clean valgrind distclean

STRIPFLAGS  = -s -R.comment -R.note -R.eh_frame -R.eh_frame_hdr

CPPFLAGS   ?= -D_LARGEFILE_SOURCE \
			  -D_LARGEFILE64_SOURCE \
			  -D_FILE_OFFSET_BITS=64 \
			  -D_POSIX_C_SOURCE=200809L \
			  -D_DEFAULT_SOURCE \
			  -D_BSD_SOURCE \
			  -DLIBNDM_SBCS_SUPPORT

CFLAGS     ?= -g3 -pipe -fPIC -std=c99 \
			  -ffunction-sections -fdata-sections -fstack-protector-all \
			  -Wall -Winit-self -Wswitch-enum -Wundef \
			  -Wmissing-field-initializers -Wconversion \
			  -Wredundant-decls -Wstack-protector -ftabstop=4 -Wshadow \
			  -Wpointer-arith -I$(PWD)/include/ \
			  -Wempty-body -Wclobbered -Waddress -Wvla -Wtype-limits

ifneq ($(UNAME),Darwin)
LDFLAGS    += -lrt
endif

CFLAGS     += -pthread
LDFLAGS    += -pthread

ifeq ($(filter sanitize,$(MAKECMDGOALS)),sanitize)
CFLAGS     += -fsanitize=address -fsanitize=leak -fsanitize=undefined
LDFLAGS    += -fsanitize=address -fsanitize=leak -fsanitize=undefined
endif

ifeq ($(filter valgrind,$(MAKECMDGOALS)),valgrind)
VG_TOOL    := `which valgrind`
VG         := $(if $(VG_TOOL),$(VG_TOOL),$(error "No valgrind executable found"))
endif

LIB_BASE   := libndm

LIB_STATIC := $(LIB_BASE).a
LIB_SHARED := $(LIB_BASE).so

ifeq ($(filter static,$(MAKECMDGOALS)),static)
LIB        := $(LIB_STATIC)
else
CFLAGS     += -fPIC
LIB        := $(LIB_SHARED)
endif

PREFIX      = /usr
EXEC_PREFIX = $(PREFIX)
LIB_DIR     = $(EXEC_PREFIX)/lib
INCLUDE_DIR = $(PREFIX)/include

OBJS        = $(patsubst %.c,%.o,$(wildcard src/*.c))
HEADERS     = $(wildcard include/ndm/*.h)

TEST_DIR    = tests
TEST_PREFIX = test_
TESTS       = $(patsubst %.c,%,$(wildcard $(TEST_DIR)/$(TEST_PREFIX)*.c))
TEST_OBJ    = $(TEST_DIR)/test.o
TEST_NO_AUTOEXEC=core core_event

EXAMPLE_DIR = examples
EXAMPLES    = $(patsubst %.c,%,$(wildcard $(EXAMPLE_DIR)/*.c))

ifeq ($(filter memory_debug,$(MAKECMDGOALS)),memory_debug)
MEMDBG_OBJ := $(TEST_DIR)/memchk.o

CPPFLAGS   += -D_MEMORY_LEAK_DEBUG -D_MEMORY_OVERFLOW_DEBUG
CFLAGS     += -include tests/memchk.h
OBJS       += $(MEMDBG_OBJ)

$(MEMDBG_OBJ): $(TEST_DIR)/memchk.c
	@echo "CC $<"
	@$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@ >/dev/null
endif

static all: $(LIB)

$(LIB_SHARED): Makefile $(HEADERS) $(OBJS)
	@echo LD $@
	@$(CC) -shared -o $@ $(OBJS) $(LDFLAGS)
	-@ls -k -1s $@

$(LIB_STATIC): Makefile $(HEADERS) $(OBJS)
	@echo AR $@
	@$(AR) rcs $@ $(OBJS)
	-@ls -k -1s $@

EXEC_TESTS_ALL = \
	$(shell find $(TEST_DIR) -name "$(TEST_PREFIX)*" -perm -u=x -type f)
EXEC_TESTS = $(filter-out $(addprefix \
	$(TEST_DIR)/$(TEST_PREFIX),$(TEST_NO_AUTOEXEC)),$(EXEC_TESTS_ALL))
EXEC_EXAMPLES_ALL = $(shell find $(EXAMPLE_DIR) -perm -u=x -type f)

sanitize valgrind check: tests
	-@for t in $(EXEC_TESTS); do echo; echo "Running $$t..."; $(VG) $$t; done

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
	@$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@ >/dev/null

$(TEST_DIR)/$(TEST_PREFIX)%: $(TEST_DIR)/$(TEST_PREFIX)%.c $(TEST_OBJ) $(LIB)
	@echo "CC $<"
	@$(CC) $< $(CPPFLAGS) $(CFLAGS) $(TEST_OBJ) $(OBJS) $(LDFLAGS) -o $@ >/dev/null

$(EXAMPLE_DIR)/%: $(EXAMPLE_DIR)/%.c $(LIB)
	@echo "CC $<"
	@$(CC) $< $(CPPFLAGS) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@ >/dev/null

%.o: %.c ../include/ndm/%.h
	@echo "CC $<"
	@$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@ >/dev/null

clean:
	rm -f src/*.o *~ *.so *.o $(LIB_STATIC) $(LIB_SHARED) $(TEST_DIR)/*.o $(EXAMPLE_DIR)/*.o
	rm -f $(EXEC_TESTS_ALL)
	rm -f $(EXEC_EXAMPLES_ALL)

distclean: clean

