CC:=$(CROSS_COMPILE)gcc

CFLAGS=-I../cryptoauthlib/lib -I../helpers -g
LDFLAGS=-L../.$(CROSS_COMPILE)build -lcryptoauth

TARGET=atecc
OBJS=atecc.o atecc-init.o atecc-config.o helpers.o ../helpers/atecc_config_zone.o

all: $(TARGET)

$(TARGET): $(OBJS)

clean:
	rm -rf $(TARGET) $(OBJS)

.PHONY: all clean
