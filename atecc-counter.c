#include "atecc-counter.h"

#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "helpers.h"
#include "basic/atca_basic.h"

#define COUNTER15_SIZE 16
#define COUNTER15_MAX_VALUE (COUNTER15_SIZE * 8)
#define COUNTER15_OFFSET 68

int do_atecc_read_counter(int argc, char **argv)
{
    if (argc < 2) {
        atecc_read_counter_help(argv[0]);
        return 1;
    }

    ATCA_STATUS status;

    uint16_t counter_id = atoi(argv[1]);
    uint32_t value = 0;
    int rev = 0;

    if (argc == 3 && !strncmp("-r", argv[2], 2)) {
        rev = 1;
    }

    if (counter_id <= 1) {
        ATECC_RETRY(status, atcab_counter_read(counter_id, &value));
        if (status != ATCA_SUCCESS) {
            eprintf("Command atcab_couter_read is failed with status 0x%x\n",
                    status);
            return 2;
        }

        if (rev) {
            value = COUNTER_MAX_VALUE - value;
        }

    } else if (counter_id == 15) {
        /* read and parse LastKeyUse section */
        uint8_t buf[COUNTER15_SIZE];
        status = atcab_read_bytes_zone(ATCA_ZONE_CONFIG, 0, COUNTER15_OFFSET,
                                        (uint8_t *) &buf, sizeof (buf));
        if (status != ATCA_SUCCESS) {
            eprintf("Command atcab_read_bytes_zone is failed with status 0x%x\n",
                    status);
            return 2;
        }

        value = 0;
        for (unsigned i = 0; i < sizeof (buf); i++) {
            while (buf[i]) {
                if (buf[i] & 0x80) {
                    value++;
                }
                buf[i] <<= 1;
            }
        }

        if (!rev) {
            value = COUNTER15_MAX_VALUE - value;
        }
    } else {
        eprintf("Unknown counter: %d\n", counter_id);
        atecc_init_counter_help(argv[0]);
        return 1;
    }

    printf("Counter %d: %u\n", counter_id, value);

    return 0;
}

void atecc_read_counter_help(const char *cmdname)
{
    eprintf("Usage: %s <counter_id> [-r]\n\n"
            "\t-r\tShow number of counts to overflow\n\n"
            "Valid counter IDs: 0, 1, 15\n", cmdname);
}

int do_atecc_increase_counter(int argc, char **argv)
{
    if (argc < 2) {
        atecc_increase_counter_help(argv[0]);
        return 1;
    }

    ATCA_STATUS status;

    uint16_t counter_id = atoi(argv[1]);
    uint32_t value = 0;

    if (counter_id <= 1) {
        ATECC_RETRY(status, atcab_counter_increment(counter_id, &value));
        if (status != ATCA_SUCCESS) {
            eprintf("Command atcab_couter_increase is failed with status 0x%x\n",
                    status);
            return 2;
        }
    } else {
        atecc_increase_counter_help(argv[0]);
        return 1;
    }

    printf("Counter %d: %u\n", counter_id, value);

    return 0;
}

void atecc_increase_counter_help(const char *cmdname)
{
    eprintf("Usage: %s <counter_id>\nValid counter IDs: 0, 1\n", cmdname);
}

void counter15_fill(uint32_t value, uint8_t *buffer)
{
    /* zero buffer first */
    memset(buffer, 0, COUNTER15_SIZE);

    for (int i = 0; i < COUNTER15_SIZE && value > 0; i++) {
        for (int j = 0; j < 8 && value > 0; j++) {
            buffer[i] >>= 1;
            buffer[i] |= 0x80;
            value--;
        }
    }
}

int do_atecc_init_counter(int argc, char **argv)
{
    if (argc < 3) {
        atecc_init_counter_help(argv[0]);
        return 1;
    }

    ATCA_STATUS status;
    uint16_t counter_id = atoi(argv[1]);
    int value = atoi(argv[2]);
    int rev = 0;

    if (value < 0) {
        rev = 1;
        value = -value;
    }

    if (counter_id <= 1) {
        if (rev) {
            value = COUNTER_MAX_VALUE - value;
        }

        ATECC_RETRY(status, atcab_write_config_counter(counter_id, value));
        if (status != ATCA_SUCCESS) {
            eprintf("Command atcab_write_config_counter is failed "
                    "with status 0x%x\n", status);
            return 2;
        }
    } else if (counter_id == 15) {
        uint8_t counter_value[COUNTER15_SIZE];

        if (value > COUNTER15_MAX_VALUE) {
            eprintf("Value is too high: %d > %d\n", value, COUNTER_MAX_VALUE);
            return 1;
        }

        if (!rev) {
            value = COUNTER15_MAX_VALUE - value;
        }

        counter15_fill(value, counter_value);

        status = atcab_write_bytes_zone(ATCA_ZONE_CONFIG, 0, COUNTER15_OFFSET,
                        counter_value, COUNTER15_SIZE);
        if (status != ATCA_SUCCESS) {
            eprintf("Command atcab_write_bytes_zone is failed "
                    "with status 0x%x\n", status);
            return 2;
        }
    } else {
        eprintf("Unknown counter: %d\n", counter_id);
        atecc_init_counter_help(argv[0]);
        return 1;
    }

    return 0;
}

void atecc_init_counter_help(const char *cmdname)
{
    eprintf("Usage: %s <counter_id> <value>\n"
            "If value is negative, sets number of counts left to overflow.\n"
            "Max value for counters 0, 1: %d\n"
            "Max value for counter 15: %d\n",
            cmdname, COUNTER_MAX_VALUE, COUNTER15_MAX_VALUE);
}
