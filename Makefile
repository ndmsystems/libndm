# Copyright (c) 2011 NDM Systems, Inc. http://www.ndmsystems.com/
# This software is freely distributable, see COPYING for details.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

NDMX_MAJOR := 0
NDMX_MINOR := 0.0
-include version.mk

VERSION=$(NDMX_MAJOR).$(NDMX_MINOR)

ifeq ($(CC),)
	CC=cc
endif

ifeq ($(STRIP),)
	STRIP=strip
endif

STRIPFLAGS=-s -R.comment -R.note
CFLAGS= -Os -fPIC -ffunction-sections -fdata-sections -I$(PWD)/include/
# -fvisibility=hidden
LDFLAGS= -Wl,--gc-sections,--relax
LIB=libndm.so
LIBV=libndm.so.$(VERSION)
LIBM=libndm.so.$(NDMX_MAJOR)
LIBS=$(LIB) $(LIBV) $(LIBM)
prefix = /usr
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include

OBJS=$(patsubst %.c,%.o,$(wildcard src/*.c))

all: $(LIBV)

$(LIBV): $(OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ $(OBJS)
#	$(STRIP) $(STRIPFLAGS) $@
	rm -f $(LIB) $(LIBM)
	ln -s $@ $(LIB)
	ln -s $@ $(LIBM)

install: $(LIBS)
	-@if [ ! -d $(exec_prefix) ]; then mkdir -p $(exec_prefix); fi
	-@if [ ! -d $(includedir)  ]; then mkdir -p $(includedir); fi
	-@if [ ! -d $(libdir)      ]; then mkdir -p $(libdir); fi
	cp -r include/ndm $(includedir)/ndm
	chmod 644 $(includedir)/ndm/*.h
	cp $(LIBS) $(libdir)
	cd $(libdir); chmod 755 $(LIBS);
	cd $(libdir); if test -f $(LIBV); then \
	  rm -f $(LIB) $(LIBM); \
	  ln -s $(LIBV) $(LIB); \
	  ln -s $(LIBV) $(LIBM); \
	  (ldconfig || true)  >/dev/null 2>&1; \
	fi

clean:
	rm -f src/*.o *~ *.so $(LIBV) $(LIBM)

%.o: %.c %.h 
