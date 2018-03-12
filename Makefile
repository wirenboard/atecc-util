CC:=$(CROSS_COMPILE)gcc

CFLAGS=-I$(abspath ../cryptoauthlib/lib) -I$(abspath ../helpers) -std=gnu99
LDFLAGS=-L$(abspath ../.$(CROSS_COMPILE)build) -lcryptoauth

TARGET=atecc
OBJS=atecc.o \
	 atecc-init.o \
	 atecc-config.o \
	 atecc-hmac.o \
	 helpers.o \
	 ../helpers/atecc_config_zone.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -rf $(TARGET) $(OBJS)

.PHONY: all clean
