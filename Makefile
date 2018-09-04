CC := gcc
CFLAGS += -lpthread -I.
LDFLAGS += -L.
INSTALL := install
PREFIX := /usr

all: test

test: test.c libapptimer.so
	$(CC) $< -o $@ -lapptimer $(CFLAGS) $(LDFLAGS)

libapptimer.so: app_timer.c
	$(CC) -fPIC -g -c -Wall $<
	$(CC) -shared -Wl,-soname,libapptimer.so -o $@ app_timer.o

install:
	$(INSTALL) libapptimer.so $(PREFIX)/lib/
	$(INSTALL) app_timer.h $(PREFIX)/include/

clean:
	$(RM) app_timer.o test libapptimer.so

.PHONY: all test install

