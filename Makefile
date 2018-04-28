USE_OPENSSL ?= y

ifdef CROSS_COMPILE
CC=$(CROSS_COMPILE)gcc
endif

ifdef DEB_HOST_GNU_TYPE
CROSS_COMPILE=$(DEB_HOST_GNU_TYPE)-
endif

CFLAGS=-I$(abspath ../cryptoauthlib/lib) -I$(abspath ../helpers) -std=gnu99
LDFLAGS=-L$(abspath ../.$(CROSS_COMPILE)build) -l:libcryptoauth.a

TARGET=atecc
OBJS=atecc.o \
	 atecc-init.o \
	 atecc-config.o \
	 atecc-hmac.o \
	 atecc-asymm.o \
	 atecc-data.o \
	 atecc-counter.o \
	 atecc-ecdh.o \
	 atecc-auth.o \
	 helpers.o \
	 util.o \
	 ../helpers/atecc_config_zone.o

# add version info from Git
GIT_VERSION:=$(shell git describe --abbrev=6 --dirty --always --tags)
CFLAGS+=-DVERSION=\"$(GIT_VERSION)\"

all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

install: $(TARGET)
	mkdir -p $(DESTDIR)/usr/bin
	install -m 0755 $(TARGET) $(DESTDIR)/usr/bin/atecc

uninstall:
	rm -rf $(DESTDIR)/usr/bin/atecc

clean:
	rm -rf $(TARGET) $(OBJS)

.PHONY: all clean install uninstall
