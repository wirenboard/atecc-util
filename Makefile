.PHONY: all libcryptoauth dist install clean

ifdef CROSS_COMPILE
CC:=$(CROSS_COMPILE)gcc
LD:=$(CROSS_COMPILE)gcc
AR:=$(CROSS_COMPILE)ar
endif

ifdef DEB_HOST_GNU_TYPE
CROSS_COMPILE=$(DEB_HOST_GNU_TYPE)-
endif

LDFLAGS=
OPTIONS := ATCAPRINTF

CRYPTOAUTHDIR := cryptoauthlib

FIND = $(shell find $(abspath $(1)) -name $(2))
MKDIR = $(shell mkdir -p $(1) 2>/dev/null)

# If the target wasn't specified assume the target is the build machine

CFLAGS=-I$(abspath cryptoauthlib/lib) -std=gnu99 -O0 -Wall -g -fPIC

# Wildcard all the sources and headers
SOURCES := $(call FIND,$(CRYPTOAUTHDIR)/lib,\*.c)
INCLUDE := $(sort $(dir $(call FIND, $(CRYPTOAUTHDIR)/lib, \*.h)))

# Gather libcryptoauth objects
LIBCRYPTOAUTH_OBJECTS := $(filter-out $(abspath $(CRYPTOAUTHDIR)/lib/hal)/%, $(SOURCES))
LIBCRYPTOAUTH_OBJECTS := $(filter-out $(abspath $(CRYPTOAUTHDIR)/lib/pkcs11)/%, $(LIBCRYPTOAUTH_OBJECTS))
LIBCRYPTOAUTH_OBJECTS := $(filter-out $(abspath $(CRYPTOAUTHDIR)/lib/openssl)/%, $(LIBCRYPTOAUTH_OBJECTS))
LIBCRYPTOAUTH_OBJECTS += $(CRYPTOAUTHDIR)/lib/hal/atca_hal.c

# General Linux Support
HAL_PREFIX := hal_linux
LIBCRYPTOAUTH_OBJECTS += $(CRYPTOAUTHDIR)/lib/hal/hal_linux_timer.c

# Native I2C hardware/driver
OPTIONS += ATCA_HAL_I2C
LIBCRYPTOAUTH_OBJECTS += $(CRYPTOAUTHDIR)/lib/hal/$(addprefix $(HAL_PREFIX),_i2c_userspace.c)

LIBCRYPTOAUTH_OBJECTS := $(LIBCRYPTOAUTH_OBJECTS:.c=.o)
CFLAGS += $(addprefix -I, $(INCLUDE) $(TEST_INCLUDE)) $(addprefix -D,$(OPTIONS))

# Regardless of platform set the vpath correctly
vpath %.c $(call BACK2SLASH,$(sort $(dir $(SOURCES) $(TEST_SOURCES))))

TARGET=atecc-util
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
	 atecc_config_zone.o

OBJS+=$(LIBCRYPTOAUTH_OBJECTS)
# add version info from Git and Debian
GIT_VERSION:=$(shell git describe --abbrev=6 --dirty --always --tags)
DEB_VERSION := $(shell head -n 1 debian/changelog  | grep -oh -P "\(\K.*(?=\))")
CFLAGS+=-DVERSION=\"$(DEB_VERSION)\ \($(GIT_VERSION)\)\"

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
	@rm -rf $(TARGET) $(OBJS)

.PHONY: all clean install uninstall
