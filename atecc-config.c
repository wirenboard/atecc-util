#include "atecc-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "helpers.h"

#include "cryptoauthlib.h"
#include "atecc_config_zone.h"

#define CONFIG_ZONE_SLOTLOCKED_OFFSET 88

void atecc_dump_config_help(const char *cmdname)
{
    eprintf("%s: dump ATECC config in human-readable format\n", cmdname);
    eprintf("Usage: %s output.txt|- [config.bin]\n", cmdname);
    eprintf("If optional third argument is set, "
            "dumps config from binary file.\n");
}

void dump_config(uint8_t config_zone[ATCA_ECC_CONFIG_SIZE])
{
    printf("============= Config zone dump: ============= \n");
    for (size_t i = 0; i < ATCA_ECC_CONFIG_SIZE; ++i) {
        if (i % 4 == 0) {
            printf("\n");
        }
        printf("%03lu: %02X\t\t", (unsigned long) i, config_zone[i]);
    }
    printf("\n");

    // printf("============= Some parsed values: ============= \n");
    // printf("Counter 0 value: %u [", parse_counter_value(config_zone + CONFIG_ZONE_COUNTER0_OFFSET));
    // for (size_t i=0; i< CONFIG_ZONE_COUNTER0_LENGTH; ++i) {
    //     printf("%02X ", config_zone[CONFIG_ZONE_COUNTER0_OFFSET + i]);
    // }
    // printf("]\n");


    // printf("Counter 1 value: %u [", parse_counter_value(config_zone + CONFIG_ZONE_COUNTER1_OFFSET));
    // for (size_t i=0; i< CONFIG_ZONE_COUNTER1_LENGTH; ++i) {
    //     printf("%02X ", config_zone[CONFIG_ZONE_COUNTER1_OFFSET + i]);
    // }
    // printf("]\n");

    uint16_t locks = config_zone[CONFIG_ZONE_SLOTLOCKED_OFFSET] | config_zone[CONFIG_ZONE_SLOTLOCKED_OFFSET + 1] << 8;

    printf("===== Individual locks configuration ==\n");
    for (size_t slot = 0; slot < ATCA_KEY_COUNT; ++slot) {
        printf(" Slot %02u: %s\n", (unsigned) slot, (locks & (1 << slot)) ? "unlocked" : "LOCKED");
    }
    printf("\n");

    printf("===== Slot configurations============\n");
    for (size_t slot = 0; slot < ATCA_KEY_COUNT; ++slot) {
        printf("=========================  Slot: %lu   ================== \n", (unsigned long) slot);
        dump_slot_config((config_zone[CONFIG_ZONE_SLOT_CONFIG_OFFSET + slot * 2 + 1] << 8) | config_zone[CONFIG_ZONE_SLOT_CONFIG_OFFSET + slot * 2]);
        printf("-------------\n");
        dump_key_config((config_zone[CONFIG_ZONE_KEY_CONFIG_OFFSET + slot * 2 + 1] << 8) | config_zone[CONFIG_ZONE_KEY_CONFIG_OFFSET + slot * 2]);
    }
}

int do_atecc_dump_config(int argc, char **argv)
{
    ATCA_STATUS status;
    uint8_t config_zone[ATCA_ECC_CONFIG_SIZE];

    if (argc != 2 && argc != 3) {
        atecc_dump_config_help(argv[0]);
        return 1;
    }

    const char *outputfilename = argv[1];
    if (strlen(outputfilename) == 0) {
        outputfilename = "-";
    }

    if (argc == 3) {  /* read from binary file */
        FILE *input = maybe_fopen(argv[2], "rb");
        if (!input) {
            perror("open config blob file for reading");
            return 1;
        }

        if (sizeof (config_zone) !=
                fread(config_zone, 1, sizeof (config_zone), input)) {
            perror("read config blob");
            maybe_fclose(input);
            return 1;
        }

        maybe_fclose(input);
    } else {
        if ((status = atcab_read_config_zone(config_zone)) != ATCA_SUCCESS) {
            eprintf("Command atcab_read_config_zone failed with status 0x%x\n",
                    status);
            return 2;
        }
    }

    int saved_stdout = maybe_set_stdout(outputfilename);
    if (saved_stdout < 0) {
        perror("open config dump file for writing");
        return 1;
    }
    dump_config(config_zone);
    maybe_restore_stdout(saved_stdout);

    return 0;
}

void atecc_read_config_help(const char *cmdname)
{
    eprintf("%s: read ATECC config zone blob into file\n", cmdname);
    eprintf("Usage: %s output.bin|-\n", cmdname);
}

int do_atecc_read_config(int argc, char **argv)
{
    int ret = 0;

    if (argc < 2) {
        atecc_read_config_help(argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    if (strlen(filename) == 0) {
        filename = "-";
    }

    FILE *out = maybe_fopen(filename, "w");
    if (!out) {
        perror("open config blob file for writing");
        return 1;
    }

    ATCA_STATUS status;
    uint8_t config_zone[ATCA_ECC_CONFIG_SIZE];

    if ((status = atcab_read_config_zone(config_zone)) != ATCA_SUCCESS) {
        eprintf("Command atcab_read_config_zone failed with status 0x%x\n",
                status);
        ret = 2;
        goto _rcexit;
    }

    if (fwrite(config_zone, 1, sizeof (config_zone), out) !=
            sizeof (config_zone)) {
        perror("write config zone blob to file");
        ret = 1;
        goto _rcexit;
    }

_rcexit:
    maybe_fclose(out);

    return ret;
}

void atecc_write_config_help(const char *cmdname)
{
    eprintf("%s: write ATECC config zone blob from file\n", cmdname);
    eprintf("Usage: %s input.bin|-\n", cmdname);
}

int do_atecc_write_config(int argc, char **argv)
{
    int ret = 0;

    if (argc < 2) {
        atecc_write_config_help(argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    if (strlen(filename) == 0) {
        filename = "-";
    }

    FILE *in = maybe_fopen(filename, "r");
    if (!in) {
        perror("open config blob file for reading");
        return 1;
    }

    ATCA_STATUS status;
    uint8_t config_zone[ATCA_ECC_CONFIG_SIZE];

    if (fread(config_zone, 1, sizeof (config_zone), in) !=
            sizeof (config_zone)) {
        perror("read config zone blob from file");
        ret = 1;
        goto _wcexit;
    }


    bool is_locked;
    status = atcab_is_locked(ATCA_ZONE_CONFIG, &is_locked);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_is_locked is failed with status 0x%x\n", status);
        ret = 2;
        goto _wcexit;
    }

    if (is_locked) {
        eprintf("Cannot configure Config Zone - Config Zone is already locked.\n");
        ret = 2;
        goto _wcexit;
    }

    status = atcab_write_config_zone(config_zone);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_write_config_zone is failed with status 0x%x\n", status);
        ret = 2;
        goto _wcexit;
    }

    // Set both counters to zero
    status = atcab_write_config_counter(0, 0);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_write_config_counter is failed with status %d\n", status);
        ret = 2;
        goto _wcexit;
    }
    status = atcab_write_config_counter(1, 0);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_write_config_counter is failed with status %d\n", status);
        ret = 2;
        goto _wcexit;
    }

_wcexit:
    maybe_fclose(in);

    return ret;
}

void atecc_lock_config_help(const char *cmdname)
{
    eprintf("%s: lock ATECC config zone. This can't be undone!\n", cmdname);
    eprintf("Usage: %s\n", cmdname);
}

int do_atecc_lock_config(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    ATCA_STATUS status;
    status = atcab_lock_config_zone();
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_lock_config_zone is failed with status 0x%x\n", status);
        return 2;
    }

    return 0;
}

void atecc_lock_slot_help(const char *cmdname)
{
    eprintf("%s: lock ATECC slot zone. This can't be undone!\n", cmdname);
    eprintf("Usage: %s <slot_id>\n", cmdname);
}

int do_atecc_lock_slot(int argc, char **argv)
{
    if (argc < 2) {
        atecc_lock_slot_help(argv[0]);
        return 1;
    }

    uint16_t slot_id = atoi(argv[1]);

    ATCA_STATUS status;
    status = atcab_lock_data_slot(slot_id);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_lock_data_slot is failed with status 0x%x\n", status);
        return 2;
    }

    return 0;
}

int do_atecc_config_is_locked(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    ATCA_STATUS status;
    bool state;

    status = atcab_is_locked(LOCK_ZONE_CONFIG, &state);

    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_is_locked is failed with status 0x%x\n", status);
        return 2;
    }

    if (state) {
        eprintf("Config zone is locked\n");
        return 0;
    } else {
        eprintf("Config zone is unlocked\n");
        return 1;
    }
}

void atecc_config_is_locked_help(const char *cmdname)
{
    eprintf("Usage: %s\nReturns 0 if config is locked, "
            "1 if unlocked, 2 on error\n", cmdname);
}

int do_atecc_extra_set(int argc, char **argv)
{
    if (argc < 3) {
        atecc_extra_set_help(argv[0]);
        return 1;
    }

    ATCA_STATUS status;
    int address = atoi(argv[1]);
    uint16_t value = atoi(argv[2]);

    value &= 0xFF;

    uint8_t mode = 0;
    if (address == 84) {
        mode = 0;
    } else if (address == 85) {
        mode = 1;
    } else {
        atecc_extra_set_help(argv[0]);
        return 1;
    }

    status = atcab_updateextra(mode, value);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_updateextra is failed with status 0x%x\n",
                status);
        return 2;
    }

    return 0;
}

void atecc_extra_set_help(const char *cmdname)
{
    eprintf("Usage: %s <address> <value>\n"
            "Writes extra byte in specific address.\n"
            "Correct addresses are 84 and 85.\n", cmdname);
}

int do_atecc_extra_get(int argc, char **argv)
{
    if (argc < 2) {
        atecc_extra_get_help(argv[0]);
        return 1;
    }

    ATCA_STATUS status;
    int address = atoi(argv[1]);
    uint8_t value;

    if (address != 84 && address != 85) {
        atecc_extra_get_help(argv[0]);
        return 1;
    }

    status = atcab_read_bytes_zone(ATCA_ZONE_CONFIG, 0, address, &value, 1);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_read_bytes_zone is failed with status 0x%x\n",
                status);
        return 2;
    }

    printf("Extra %02d: 0x%02hhx\n", address, value);

    return 0;
}

void atecc_extra_get_help(const char *cmdname)
{
    eprintf("Usage: %s <address>\n"
            "Reads extra byte from specific address.\n"
            "Correct addresses are 84 and 85.\n", cmdname);
}
