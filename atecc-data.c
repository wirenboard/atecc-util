#include "atecc-data.h"

#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"
#include "util.h"
#include "basic/atca_basic.h"

int do_atecc_write_data(int argc, char **argv)
{
    if (argc < 4) {
        atecc_write_data_help(argv[0]);
        return 1;
    }

    int ret = 0;
    int slot_id = atoi(argv[1]);
    int offset = atoi(argv[2]);
    const char *inputfilename = argv[3];
    FILE *inputfile = NULL;
    uint8_t *inputbuffer = NULL;
    size_t slotsize = 0, inputsize = 0, readsize = 0;
    ATCA_STATUS status;

    /* check offset */
    if (offset % ATCA_WORD_SIZE != 0) {
        eprintf("Offset must be aligned to %d!\n", ATCA_WORD_SIZE);
        return 1;
    }

    /* get size of specific slot */
    ATECC_RETRY(status, atcab_get_zone_size(ATCA_ZONE_DATA, slot_id, &slotsize));
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_get_zone_size is failed with status 0x%x\n",
                status);
        return 2;
    }

    inputbuffer = (uint8_t *) malloc(slotsize);
    if (!inputbuffer) {
        perror("allocating slot buffer");
        return 1;
    }

    inputfile = maybe_fopen(inputfilename, "rb");
    if (!inputfile) {
        perror("open data file for reading");
        free(inputbuffer);
        return 1;
    }

    /* read as much data as possible */
    while ((readsize = fread(inputbuffer + inputsize, 1,
                slotsize - offset - inputsize, inputfile)) != 0) {
        inputsize += readsize;
    }

    /* check input size */
    if (!feof(inputfile)) {
        eprintf("File doesn't fit in slot area: "
                "slot size %lu\n", slotsize);
        ret = 1;
        goto _exit;
    }

    if (inputsize % ATCA_WORD_SIZE != 0) {
        eprintf("Input size must be aligned to word size %d\n",
                ATCA_WORD_SIZE);
        ret = 1;
        goto _exit;
    }

    /* try to write data to chip */
    status = atcab_write_bytes_zone(ATCA_ZONE_DATA, slot_id, offset,
                inputbuffer, inputsize);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_write_bytes_zone is failed with status 0x%x\n",
                status);
        ret = 2;
    }

_exit:

    if (inputbuffer) {
        free(inputbuffer);
    }

    maybe_fclose(inputfile);

    return ret;
}

void atecc_write_data_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> <offset> input_file\n", cmdname);
    eprintf("Writes data from file to specific slot with offset.\n"
            "Data is written as plaintext.\n");
}

int do_atecc_read_data(int argc, char **argv)
{
    if (argc < 5) {
        atecc_read_data_help(argv[0]);
        return 1;
    }

    int ret = 0;
    int slot_id = atoi(argv[1]);
    int offset = atoi(argv[2]);
    size_t outputsize = atoi(argv[3]);
    const char *outputfilename = argv[4];
    FILE *outputfile = NULL;
    uint8_t *outputbuffer = NULL;
    ATCA_STATUS status;

    uint8_t readkey[ATCA_KEY_SIZE];
    const char *readkeyfilename = NULL;
    uint16_t readkey_slot;

    if (argc == 7) {
        readkeyfilename = argv[5];
        readkey_slot = atoi(argv[6]);

        size_t readsize;
        FILE *readkeyfile = maybe_fopen(readkeyfilename, "rb");
        if (!readkeyfile) {
            perror("open read key file for reading");
            return 1;
        }

        readsize = fread(readkey, 1, sizeof (readkey), readkeyfile);
        if (readsize != sizeof (readkey)) {
            perror("read read key from file");
            maybe_fclose(readkeyfile);
            return 1;
        }

        maybe_fclose(readkeyfile);
    }

    /* read data from ATECC */
    if (readkeyfilename != NULL) {
        outputsize = min(outputsize, ATCA_KEY_SIZE);
        outputbuffer = (uint8_t *) malloc(ATCA_KEY_SIZE);
        if (!outputbuffer) {
            perror("allocating slot buffer");
            return 1;
        }

        ATECC_RETRY(status, atcab_read_enc(slot_id, offset, outputbuffer,
                    readkey, readkey_slot));

        if (status != ATCA_SUCCESS) {
            eprintf("Command atcab_read_enc is failed with status 0x%x\n",
                    status);
            free(outputbuffer);
            return 2;
        }
    } else {
        outputbuffer = (uint8_t *) malloc(outputsize);
        if (!outputbuffer) {
            perror("allocating slot buffer");
            return 1;
        }

        ATECC_RETRY(status, atcab_read_bytes_zone(ATCA_ZONE_DATA, slot_id, offset,
                        outputbuffer, outputsize));

        if (status != ATCA_SUCCESS) {
            eprintf("Command atcab_read_bytes_zone is failed with status 0x%x\n",
                    status);
            free(outputbuffer);
            return 2;
        }
    }

    outputfile = maybe_fopen(outputfilename, "wb");
    if (!outputfile) {
        perror("open data file for writing");
        free(outputbuffer);
        return 1;
    }

    if (outputsize != fwrite(outputbuffer, 1, outputsize,
                outputfile)) {
        perror("write data to output file");
        ret = 1;
    }

    if (outputbuffer) {
        free(outputbuffer);
    }

    maybe_fclose(outputfile);

    return ret;
}

void atecc_read_data_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> <offset> <size> output_file "
            "[readkey_file <readkey_slot>]\n", cmdname);
    eprintf("Reads data from specific slot with offset.\n"
            "If keys are not set, data is read as plaintext.\n");
}

int do_atecc_lock_data(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    ATCA_STATUS status;

    ATECC_RETRY(status, atcab_lock_data_zone());
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_lock_data_zone is failed with status 0x%x\n",
                status);
        return 2;
    }

    return 0;
}

void atecc_lock_data_help(const char *cmdname)
{
    eprintf("Usage: %s\nLocks data zone. This can't be undone!\n",
            cmdname);
}

int do_atecc_data_is_locked(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    ATCA_STATUS status;
    bool state;

    ATECC_RETRY(status, atcab_is_locked(LOCK_ZONE_DATA, &state));

    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_is_locked is failed with status 0x%x\n", status);
        return 2;
    }

    if (state) {
        eprintf("Data zone is locked\n");
        return 0;
    } else {
        eprintf("Data zone is unlocked\n");
        return 1;
    }
}

void atecc_data_is_locked_help(const char *cmdname)
{
    eprintf("Usage: %s\nReturns 0 if data is locked, "
            "1 if unlocked, 2 on error\n", cmdname);
}

int do_atecc_slot_is_locked(int argc, char **argv)
{
    if (argc < 2) {
        atecc_slot_is_locked_help(argv[0]);
        return 2;
    }

    uint16_t slot = atoi(argv[1]);

    ATCA_STATUS status;
    bool state;

    ATECC_RETRY(status, atcab_is_slot_locked(slot, &state));

    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_is_slot_locked is failed with status 0x%x\n", status);
        return 2;
    }

    if (state) {
        eprintf("Slot %hu is locked\n", slot);
        return 0;
    } else {
        eprintf("Slot %hu is unlocked\n", slot);
        return 1;
    }
}

void atecc_slot_is_locked_help(const char *cmdname)
{
    eprintf("Usage: %s <slot>\nReturns 0 if slot is locked, "
            "1 if unlocked, 2 on error\n", cmdname);
}
