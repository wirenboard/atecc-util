atecc-util
================

Linux command-line tool for ATECC608A and ATECC508A ICs connected via i2c.
Uses Microchip CryptoAuthLib.

## Overview

atecc-util consists of set of distinct tools called commands. Each command have its own argument format. There are commands for:
* reading serial number
* reading and manipulating config zone
* calculating HMAC
* generating, writing and reading ECC keys, ECDSA signing and verifying 
* writing and reading data to/from slots
* locking data zone and individual data slots
* reading and manipulating counters
* reading and writing extra bytes
* ECDH
* password authentification

Multiple commands can be specified at once as following:

    atecc-util -b 10 -c 'serial' -c 'read-config /tmp/config.dump'

## Installation


Please use pre-built Debian packages when possible.

atecc-util don't have external dependencies besides Microchip's CryptoAuthLib which is included as a submodule. 

clone this repo:

    git clone https://github.com/contactless/atecc-util
    cd atecc-util

Initialize submodules

    git submodule init
    git submodule update

Run GNU Make

    make

You can build Debian package as usual:

    dpkg-buildpackage


## List of commands


### serial

Reads ATECCx08 IC serial number in hex format:
```
$ ./atecc-util -b 10 -c 'serial'
012355b52d66a109ee
```

### write-config

write ATECC config zone blob from file

Usage: `write-config input.bin|-`

### read-config
read ATECC config zone blob into file

Usage: `read-config output.bin|-`

### dump-config

dump ATECC config in human-readable format

Usage: `dump-config output.txt|- [config.bin]`

If optional third argument is set, dumps config from binary file.

### lock-config

lock ATECC config zone. This can't be undone!

Usage: `lock-config`

### config-is-locked
Usage: `config-is-locked`

Returns 0 if config is locked, 1 if unlocked, 2 on error

### hmac-write-key

Usage: `hmac-write-key <slot_id> <offset> data_file [write_key <write_key_id>]`

`slot_id`	ID of slot to write data block to


`offset` Offset (in 32-byte blocks) to write data block to

`keyfile`	File containing data block to write (32 bytes long)

`write_key`	File containing write-guarding key (32 bytes long)

`write_key_id`	ID of write key on device
If both data and readkey must be read from stdin, key is read first.

### hmac-dgst

Usage: `hmac-dgst <slot_id> <payload_file> <hmac_output>`

`slot_id`	ID of slot to use for HMAC dgstulation

`payload_file`	File with payload (or - for stdin)

`hmac_output`	HMAC output file (or - for stdout)


### ecc-gen

Usage: `ecc-gen <slot_id> [pubkey_file]`

Generates an ECDSA private key in given slot.
If pubkey_file is set, also writes public key into file.

### ecc-write

Usage: `ecc-write <slot_id> private_key_file [<write_key_slot> write_key_file]`

Writes an ECDSA private key in given slot.
Private key is 32 bytes in length, without 4 leading zeroes.
If data section is locked, you also need to determine write key.

### ecc-read-pub

Usage: `ecc-read-pub <slot_id> pubkey_file`

Reads a public key from selected slot. Note that 
only slots 8 to 15 are large enough for a public key.
Output format: 32 bytes of X and Y, big-endian

### ecc-gen-pub

Usage: `ecc-gen-pub <slot_id> pubkey_file`

Generates a public key from private in selected slot.
Output format: 32 bytes of X and Y, big-endian

### ecc-sign

Usage `ecc-sign <slot_id> message_file signature_file`

Calculates a signature for message using private key in given slot

### ecc-verify

Usage `ecc-verify <slot_id> message_file signature_file [pubkey]`

Verifies a signature for message using public key in given slot

### write-data

Usage: `write-data <slot_id> <offset> input_file`

Writes data from file to specific slot with offset.
Data is written as plaintext.

### write-data-block

Usage: `write-data-block <slot_id> <offset> data_file [write_key <write_key_id>]`

`slot_id`	ID of slot to write data block to

`offset`	Offset (in 32-byte blocks) to write data block to

`keyfile`	File containing data block to write (32 bytes long)

`write_key`	File containing write-guarding key (32 bytes long)

`write_key_id`	ID of write key on device


If both data and readkey must be read from stdin, key is read first.### read-data

Usage: `read-data <slot_id> <offset> <size> output_file [readkey_file <readkey_slot>]`

Reads data from specific slot with offset.
If keys are not set, data is read as plaintext.

### lock-data

Usage: `lock-data`

Locks data zone. This can't be undone!

### data-is-locked

Usage: `data-is-locked`

Returns 0 if data is locked, 1 if unlocked, 2 on error

### lock-slot

lock-slot: lock ATECC slot zone. This can't be undone!
Usage: `lock-slot <slot_id>`


### slot-is-locked

Usage: `slot-is-locked <slot>`

Returns 0 if slot is locked, 1 if unlocked, 2 on error

### counter-read

Usage: `counter-read <counter_id> [-r]`

`-r`	Show number of counts to overflow

Valid counter IDs: 0, 1, 15

### counter-inc

Usage: `counter-inc <counter_id>`

Valid counter IDs: 0, 1

### counter-init

Usage: `counter-init <counter_id> <value>`

If value is negative, sets number of counts left to overflow.
Max value for counters 0, 1: 2097151
Max value for counter 15: 128

### extra-set

Usage: `extra-set <address> <value>`

Writes extra byte in specific address.
Correct addresses are 84 and 85.

### extra-get

Usage: `extra-get <address>`

Reads extra byte from specific address.
Correct addresses are 84 and 85.

### ecdh

Usage: `ecdh <slot_id> public_key [secret_file]`

`slot_id`	ID of slot with private key

`public_key`	File with public key (64 bytes, big-endian

`secret_file`	Optional file to store shared secret
If slot is configured to save secret in next slot, no secret is returned.

### auth-passwd

Usage: `auth-passwd <slot_id> password_file`

Authorizes key to use in next commands in row using password.

`slot_id`	ID of slot with authorizing key.

`password_file`	Input stream with password (file or stdin).

Password ends with EOF or newline. Max length of password is 256

### auth-make-passwd

Usage: `auth-make-passwd <slot_id> password_file`

Makes a key from password.

`slot_id`	ID of slot to write a key.

`password_file`	Input stream with password (file or stdin).

Password ends with EOF or newline. Max length of password is 256

### auth-check-gendig

Usage: `auth-check-gendig <slot_id> key_file`

Checks key in selected slot matches key in file using GenDig.
