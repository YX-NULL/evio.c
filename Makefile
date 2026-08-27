# evio.c Makefile
# Builds: shared library (libevio.so), static library (libevio.a), test programs
#
# Usage:
#   make              - build shared + static library
#   make test         - build and run evio_test (original test)
#   make boundary     - build and run evio_boundary_test
#   make all          - build library + all test programs
#   make clean        - remove build artifacts

CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -fPIC
LDFLAGS ?=
LDLIBS  := -lpthread

SRCS     := evio.c buf.c
OBJS     := $(SRCS:.c=.o)
LIB_SONAME := libevio.so
LIB_SHARED   := libevio.so.1
LIB_STATIC   := libevio.a

PREFIX  ?= /usr/local
BINDIR  := $(PREFIX)/bin
LIBDIR  := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include

.PHONY: all test boundary clean install

all: $(LIB_SHARED) $(LIB_STATIC)

# ── Object files ──────────────────────────────────────────────
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ── Shared library ────────────────────────────────────────────
$(LIB_SHARED): $(OBJS)
	$(CC) -shared -Wl,-soname,$(LIB_SONAME) -o $@ $^ $(LDLIBS)

# ── Static library ────────────────────────────────────────────
$(LIB_STATIC): $(OBJS)
	ar rcs $@ $^

# ── Original test (built with EVIO_TEST flag) ────────────────
evio_test: evio.c buf.c evio_test.c
	$(CC) $(CFLAGS) -DEVIO_TEST -o $@ evio.c buf.c evio_test.c $(LDLIBS)

test: evio_test
	./evio_test

# ── Boundary test ─────────────────────────────────────────────
evio_boundary_test: evio.c buf.c evio_boundary_test.c
	$(CC) $(CFLAGS) -DEVIO_BOUNDARY_TEST -o $@ evio.c buf.c evio_boundary_test.c $(LDLIBS)

boundary: evio_boundary_test
	./evio_boundary_test

# ── Install ───────────────────────────────────────────────────
install: $(LIB_SHARED) $(LIB_STATIC)
	install -d $(INCLUDEDIR) $(LIBDIR)
	install -m 644 evio.h buf.h $(INCLUDEDIR)/
	install -m 755 $(LIB_SHARED) $(LIBDIR)/
	install -m 644 $(LIB_STATIC) $(LIBDIR)/
	ln -sf $(LIB_SHARED) $(LIBDIR)/$(LIB_SONAME)
	ln -sf $(LIB_SHARED) $(LIBDIR)/libevio.so

# ── Clean ─────────────────────────────────────────────────────
clean:
	rm -f *.o $(LIB_SHARED) $(LIB_STATIC) evio_test evio_boundary_test tbound.sock tsock echo.sock
