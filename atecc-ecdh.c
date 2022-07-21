#include "atecc-ecdh.h"
#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "basic/atca_basic.h"

int do_atecc_ecdh(int argc, char **argv)
{
    if (argc < 3) {
        atecc_ecdh_help(argv[0]);
        return 1;
    }

    uint16_t slot_id = atoi(argv[1]);
    uint8_t pms[ATCA_KEY_SIZE];
    uint8_t pubkey[ATCA_PUB_KEY_SIZE];
    size_t read_size = 0;
    ATCA_STATUS status;

    FILE *pubkeyfile = maybe_fopen(argv[2], "rb");
    if (!pubkeyfile) {
        perror("open public key file for reading");
        return 1;
    }

    read_size = fread(pubkey, 1, sizeof (pubkey), pubkeyfile);
    if (read_size != sizeof (pubkey)) {
        perror("read pubkey from file");
        maybe_fclose(pubkeyfile);
        return 1;
    }

    maybe_fclose(pubkeyfile);

    // Force send ATECC to idle mode, so watchdog won't be triggered
    // in the middle of command execution.
    // This may happen because of I/O in fread slow enough,
    // it adds delay between ATECC init sequence in main() and this operation.
    status = atcab_idle();
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_idle is failed with status 0x%x\n", status);
        return 2;
    }

    ATECC_RETRY(status, atcab_ecdh(slot_id, pubkey, pms));
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_ecdh is failed with status 0x%x\n", status);
        return 2;
    }

    if (argc == 4) {
        size_t write_size;
        FILE *pmsfile = maybe_fopen(argv[3], "wb");
        if (!pmsfile) {
            perror("open secret file for writing");
            return 1;
        }

        write_size = fwrite(pms, 1, sizeof (pms), pmsfile);
        if (write_size != sizeof (pms)) {
            perror("write secret to file");
            maybe_fclose(pmsfile);
            return 1;
        }

        maybe_fclose(pmsfile);
    }

    return 0;
}

void atecc_ecdh_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> public_key [secret_file]\n"
            "\tslot_id\tID of slot with private key\n"
            "\tpublic_key\tFile with public key (64 bytes, big-endian\n"
            "\tsecret_file\tOptional file to store shared secret\n"
            "If slot is configured to save secret in next slot, "
            "no secret is returned.\n", cmdname);
}
