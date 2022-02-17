#include "atecc-hmac.h"
#include "helpers.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>

#include "cryptoauthlib.h"

int do_atecc_hmac_write_key(int argc, char **argv)
{
    if (argc < 4 || argc == 5) {
        atecc_hmac_write_key_help(argv[0]);
        return 1;
    }

    int slot_id = atoi(argv[1]);
    int offset = atoi(argv[2]);
    const char *keyfilename = argv[3];
    const char *writekeyfilename = NULL;
    uint16_t key_id = 0;

    if (argc == 6) {
        writekeyfilename = argv[4];
        key_id = atoi(argv[5]);
    }
    size_t readlen = 0;

    uint8_t writekey[HMAC_KEY_SIZE];
    if (writekeyfilename) {
        FILE *writekeyfile = maybe_fopen(writekeyfilename, "rb");
        if (!writekeyfile) {
            perror("open writekey file");
            return 1;
        }

        readlen = fread(writekey, 1, sizeof (writekey), writekeyfile);
        if (readlen != sizeof (writekey)) {
            perror("read write key from file");
            maybe_fclose(writekeyfile);
            return 1;
        }

        maybe_fclose(writekeyfile);
    }

    FILE *keyfile = maybe_fopen(keyfilename, "rb");
    if (!keyfile) {
        perror("open keyfile");
        return 1;
    }

    uint8_t key[HMAC_KEY_SIZE];
    readlen = fread(key, 1, sizeof (key), keyfile);
    if (readlen != sizeof (key)) {
        perror("read key from file");
        maybe_fclose(keyfile);
        return 1;
    }

    maybe_fclose(keyfile);

    ATCA_STATUS status;
    const char *cmd;
    if (!writekeyfilename) {
        cmd = "atcab_write_zone";
        ATECC_RETRY(status, atcab_write_zone(ATCA_ZONE_DATA, slot_id, offset, 0, key, 32));
        if (status != ATCA_SUCCESS) {
            eprintf("Command %s is failed with status 0x%x\n", cmd, status);
            return 2;
        }
    } else {
        cmd = "atcab_write_enc";
        ATECC_RETRY(status, atcab_write_enc(slot_id, offset, key, writekey, key_id));
        if (status != ATCA_SUCCESS) {
            eprintf("Command %s is failed with status 0x%x\n", cmd, status);
            return 2;
        }
    }

    return 0;
}

void atecc_hmac_write_key_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> <offset> data_file [write_key <write_key_id>]\n", cmdname);
    eprintf("\tslot_id\tID of slot to write data block to\n");
    eprintf("\toffset\tOffset (in 32-byte blocks) to write data block to\n");
    eprintf("\tkeyfile\tFile containing data block to write (32 bytes long)\n");
    eprintf("\twrite_key\tFile containing write-guarding key (32 bytes long)\n");
    eprintf("\twrite_key_id\tID of write key on device\n");
    eprintf("If both data and readkey must be read from stdin, "
            "key is read first.");
}

int do_atecc_hmac_dgst(int argc, char **argv)
{
    if (argc < 4) {
        atecc_hmac_dgst_help(argv[0]);
        return 1;
    }

    int slot_id = atoi(argv[1]);
    const char *payloadfilename = argv[2];
    const char *hmacfilename = argv[3];
    uint8_t buffer[PAYLOAD_BUFFER_SIZE];
    uint8_t hmac[ATCA_SHA_DIGEST_SIZE];
    ATCA_STATUS status;
    atca_hmac_sha256_ctx_t ctx;

    FILE *payloadfile = maybe_fopen(payloadfilename, "rb");
    if (!payloadfile) {
        perror("open payload file for reading");
        return 1;
    }

    /* send payload to ATECC */
    ATECC_RETRY(status, atcab_sha_hmac_init(&ctx, slot_id));
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_sha_hmac_init is failed with status 0x%x\n", status);
        maybe_fclose(payloadfile);
        return 2;
    }

    while (!feof(payloadfile)) {
        size_t sz;

        if (sizeof (buffer) !=
                (sz = fread(buffer, 1, sizeof (buffer), payloadfile))) {
            if (!feof(payloadfile)) {
                perror("reading from payload file");
                maybe_fclose(payloadfile);
                return 1;
            }
        }

        ATECC_RETRY(status, atcab_sha_hmac_update(&ctx, buffer, sz));
        if (status != ATCA_SUCCESS) {
            eprintf("Command atcab_sha_hmac_update is failed with status 0x%x\n", status);
            return status;
        }
    }
    maybe_fclose(payloadfile);

    ATECC_RETRY(status, atcab_sha_hmac_finish(&ctx, hmac, SHA_MODE_TARGET_TEMPKEY));
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_sha_hmac_finish is failed with status %d\n", status);
        return status;
    }

    FILE *hmacfile = maybe_fopen(hmacfilename, "wb");
    if (!hmacfile) {
        perror("open hmac file for writing");
        return 1;
    }

    if (fwrite(hmac, 1, sizeof (hmac), hmacfile) != sizeof (hmac)) {
        perror("write hmac to file");
        maybe_fclose(hmacfile);
        return 1;
    }

    maybe_fclose(hmacfile);

    return 0;
}

void atecc_hmac_dgst_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> <payload_file> <hmac_output>\n", cmdname);
    eprintf("\tslot_id\tID of slot to use for HMAC dgstulation\n");
    eprintf("\tpayload_file\tFile with payload (or - for stdin)\n");
    eprintf("\thmac_output\tHMAC output file (or - for stdout)\n");
}
