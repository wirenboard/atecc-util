#include "atecc-asymm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"
#include "util.h"

#include "basic/atca_basic.h"

int do_atecc_gen_private(int argc, char **argv)
{
    if (argc < 2) {
        atecc_gen_private_help(argv[0]);
        return 1;
    }

    ATCA_STATUS status;
    int key_id = atoi(argv[1]);
    const char *pubkeyfilename = NULL;
    FILE *pubkeyfile = NULL;
    uint8_t pubkeybuffer[ATCA_PUB_KEY_SIZE];
    uint8_t *pubkey = NULL;

    if (argc == 3) {
        pubkeyfilename = argv[2];
        pubkey = pubkeybuffer;
        pubkeyfile = maybe_fopen(pubkeyfilename, "wb");
        if (!pubkeyfile) {
            perror("open public key file for writing");
            return 1;
        }
    }

    status = atcab_genkey(key_id, pubkey);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_genkey is failed with status 0x%x\n", status);
        if (pubkeyfile) {
            maybe_fclose(pubkeyfile);
        }
        return 2;
    }

    if (pubkeyfile) {
        maybe_fclose(pubkeyfile);
    }

    return 0;
}

void atecc_gen_private_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> [pubkey_file]\n", cmdname);
    eprintf("Generates an ECDSA private key in given slot.\n");
    eprintf("If pubkey_file is set, also writes public key into file.\n");
}

int do_atecc_write_private(int argc, char **argv)
{
    if (argc != 3 && argc != 5) {
        atecc_write_private_help(argv[0]);
        return 1;
    }

    ATCA_STATUS status;
    int key_id = atoi(argv[1]);
    uint8_t privatekey[ATCA_PRIV_KEY_SIZE];
    const char *privatekeyfilename = argv[2];
    FILE *privatekeyfile = NULL, *writekeyfile = NULL;

    int writekey_id = -1;
    uint8_t writekeybuffer[ATCA_KEY_SIZE];
    uint8_t *writekey = NULL;
    const char *writekeyfilename = NULL;

    if (argc == 5) {
        writekey = writekeybuffer;
        writekey_id = atoi(argv[3]);
        writekeyfilename = argv[4];

        /* read write key */
        writekeyfile = maybe_fopen(writekeyfilename, "rb");
        if (!writekeyfile) {
            perror("open write key for reading");
            return 1;
        }

        if (sizeof (writekeybuffer) !=
                fread(writekeybuffer, 1, sizeof (writekeybuffer), writekeyfile)) {
            perror("read write key from file");
            maybe_fclose(writekeyfile);
            return 1;
        }

        maybe_fclose(writekeyfile);
    }

    /* read private key */
    privatekeyfile = maybe_fopen(privatekeyfilename, "rb");
    if (!privatekeyfile) {
        perror("open private key file for reading");
        return 1;
    }

    if (sizeof (privatekey) !=
            fread(privatekey, 1, sizeof (privatekey), privatekeyfile)) {
        perror("read private key from file");
        maybe_fclose(privatekeyfile);
        return 1;
    }

    maybe_fclose(privatekeyfile);

    status = atcab_priv_write(key_id, privatekey, writekey_id, writekey);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_priv_write is failed with status 0x%x\n", status);
        return 2;
    }

    return 0;
}

void atecc_write_private_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> private_key_file "
            "[<write_key_slot> write_key_file]\n", cmdname);
    eprintf("Writes an ECDSA private key in given slot.\n");
    eprintf("If data section is locked, you also need to "
            "determine write key.\n");
}

int do_atecc_read_pub(int argc, char **argv)
{
    ATCA_STATUS status;
    if (argc < 3) {
        atecc_read_pub_help(argv[0]);
        return 1;
    }

    int slot_id = atoi(argv[1]);
    const char *outfile = argv[2];
    uint8_t buffer[ATCA_PUB_KEY_SIZE];

    FILE *pubkey = maybe_fopen(outfile, "wb");
    if (!pubkey) {
        perror("open public key file for writing");
        return 1;
    }

    status = atcab_read_pubkey(slot_id, buffer);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_read_pubkey is failed with status 0x%x\n", status);
        maybe_fclose(pubkey);
        return 2;
    }

    if (sizeof (buffer) !=
            fwrite(buffer, 1, sizeof (buffer), pubkey)) {
        perror("write public key in file");
        maybe_fclose(pubkey);
        return 1;
    }

    maybe_fclose(pubkey);

    return 0;
}

void atecc_read_pub_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> pubkey_file\n", cmdname);
    eprintf("Reads a public key from selected slot. Note that \n"
            "only slots 8 to 15 are large enough for a public key.\n");
    eprintf("Output format: 32 bytes of X and Y, big-endian\n");
}

int do_atecc_gen_pub(int argc, char **argv)
{
    ATCA_STATUS status;
    if (argc < 3) {
        atecc_gen_pub_help(argv[0]);
        return 1;
    }

    int slot_id = atoi(argv[1]);
    const char *outfile = argv[2];
    uint8_t buffer[ATCA_PUB_KEY_SIZE];

    FILE *pubkey = maybe_fopen(outfile, "wb");
    if (!pubkey) {
        perror("open public key file for writing");
        return 1;
    }

    status = atcab_get_pubkey(slot_id, buffer);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_get_pubkey is failed with status 0x%x\n", status);
        maybe_fclose(pubkey);
        return 2;
    }

    if (sizeof (buffer) !=
            fwrite(buffer, 1, sizeof (buffer), pubkey)) {
        perror("write public key in file");
        maybe_fclose(pubkey);
        return 1;
    }

    maybe_fclose(pubkey);

    return 0;
}

void atecc_gen_pub_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> pubkey_file\n", cmdname);
    eprintf("Generates a public key from private in selected slot.\n");
    eprintf("Output format: 32 bytes of X and Y, big-endian\n");
}

int do_atecc_sign(int argc, char **argv)
{
    if (argc < 4) {
        atecc_sign_help(argv[0]);
        return 1;
    }

    int slot_id = atoi(argv[1]);
    const char *messagefilename = argv[2];
    const char *signaturefilename = argv[3];
    ATCA_STATUS status;

    FILE *signaturefile = NULL;
    uint8_t signature[ATCA_SIG_SIZE];
    uint8_t digest[ATCA_SHA2_256_DIGEST_SIZE];

    if (sha256_file(messagefilename, digest)) {
        return 1;
    }

    status = atcab_sign(slot_id, digest, signature);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_sign is failed with status 0x%x\n", status);
        return 2;
    }

    signaturefile = maybe_fopen(signaturefilename, "wb");
    if (!signaturefile) {
        perror("open signature file for writing");
        return 1;
    }

    if (fwrite(signature, 1, sizeof (signature), signaturefile) !=
            sizeof (signature)) {
        perror("write signature to file");
        maybe_fclose(signaturefile);
        return 1;
    }

    maybe_fclose(signaturefile);

    return 0;
}

void atecc_sign_help(const char *cmdname)
{
    eprintf("Usage %s <slot_id> message_file signature_file\n", cmdname);
    eprintf("Calculates a signature for message using "
            "private key in given slot\n");
}

int do_atecc_verify(int argc, char **argv)
{
    if (argc < 4) {
        atecc_verify_help(argv[0]);
        return 2;
    }

    uint16_t slot_id = atoi(argv[1]);
    const char *messagefilename = argv[2];
    const char *signaturefilename = argv[3];

    const char *pubkeyfilename = NULL;
    uint8_t pubkey[ATCA_PUB_KEY_SIZE];

    if (argc == 5) {
        pubkeyfilename = argv[4];

        FILE *pubkeyfile = fopen(pubkeyfilename, "rb");
        if (!pubkeyfile) {
            perror("open public key file for reading");
            return 2;
        }

        if (fread(pubkey, 1, sizeof (pubkey), pubkeyfile) !=
                    sizeof (pubkey)) {
            perror("read pubkey from file");
            fclose(pubkeyfile);
            return 2;
        }

        fclose(pubkeyfile);
    }

    uint8_t signature[ATCA_SIG_SIZE];
    uint8_t digest[ATCA_SHA2_256_DIGEST_SIZE];
    bool verified = false;

    ATCA_STATUS status;

    if (sha256_file(messagefilename, digest)) {
        return 2;
    }

    FILE *signaturefile = maybe_fopen(signaturefilename, "rb");
    if (!signaturefile) {
        perror("open signature file for reading");
        return 2;
    }

    if (fread(signature, 1, sizeof (signature), signaturefile) !=
            sizeof (signature)) {
        perror("read signature from file");
        return 2;
    }

    maybe_fclose(signaturefile);

    if (!pubkeyfilename) {
        status = atcab_verify_stored(digest, signature, slot_id, &verified);
        if (status != ATCA_SUCCESS) {
            eprintf("Command atcab_verify_stored is failed with status 0x%x\n",
                    status);
            return 2;
        }
    } else {
        status = atcab_verify_extern(digest, signature, pubkey, &verified);
        if (status != ATCA_SUCCESS) {
            eprintf("Command atcab_verify_extern is failed with status 0x%x\n",
                    status);
            return 2;
        }
    }

    if (verified) {
        eprintf("Verification OK\n");
        return 0;
    } else {
        eprintf("Verification FAILED\n");
        return 1;
    }
}

void atecc_verify_help(const char *cmdname)
{
    eprintf("Usage %s <slot_id> message_file signature_file [pubkey]\n", cmdname);
    eprintf("Verifies a signature for message using "
            "public key in given slot\n");
}
