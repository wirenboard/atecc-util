#include "atecc-auth.h"

#include "helpers.h"
#include "util.h"
#include "basic/atca_basic.h"
#include "host/atca_host.h"
#include "crypto/atca_crypto_sw.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PASSWD_LEN 256
#define PASSWD_SALT "atecc-salt123"

static int read_passwd(const char *filename, char *passwd)
{
    FILE *f = maybe_fopen(filename, "r");
    if (!f) {
        perror("open password file for reading");
        return 0;
    }

    if (fgets(passwd, MAX_PASSWD_LEN + 2, f) == NULL) {
        perror("read password from file");
        maybe_fclose(f);
        return 0;
    }

    maybe_fclose(f);

    /* remove trailing newline if it is there */
    int tail = strlen(passwd) - 1;
    if (passwd[tail] == '\n') {
        passwd[tail] = '\0';
    }

    return 1;
}

static int make_key_from_password(const char *password, uint8_t key[ATCA_KEY_SIZE])
{
    uint8_t buffer[MAX_PASSWD_LEN + sizeof (PASSWD_SALT) + 2];

    strcpy((char *) buffer, password);
    strcpy((char *) buffer + strlen(password), PASSWD_SALT);

    if (atcac_sw_sha2_256((const uint8_t *) buffer, strlen((char *) buffer), key) != 0) {
        return 0;
    }

    return 1;
}

static int read_key(const char *filename, uint8_t key[ATCA_KEY_SIZE])
{
    FILE *f = maybe_fopen(filename, "rb");
    if (!f) {
        perror("open key file for reading");
        return 0;
    }

    if (ATCA_KEY_SIZE != fread(key, 1, ATCA_KEY_SIZE, f)) {
        perror("read key from file");
        maybe_fclose(f);
        return 0;
    }

    maybe_fclose(f);

    return 1;
}

int do_atecc_auth_passwd(int argc, char **argv)
{
    if (argc < 3) {
        atecc_auth_passwd_help(argv[0]);
        return 2;
    }

    uint16_t slot_id = atoi(argv[1]);
    const char *passwd_file = argv[2];

    /* read password */
    char passwd[MAX_PASSWD_LEN + 2];
    if (!read_passwd(passwd_file, passwd)) {
        return 2;
    }

    /* make a key from password */
    uint8_t key[ATCA_KEY_SIZE];
    if (!make_key_from_password(passwd, key)) {
        return 2;
    }

    /* generate MAC of specific message */
    uint8_t sn[9];
    uint8_t num_in[NONCE_NUMIN_SIZE] = { 0 };
    uint8_t rand_out[RANDOM_NUM_SIZE] = { 0 };
    uint8_t other_data[12] = { 0 };
    uint8_t resp[32];
    ATCA_STATUS status;

    status = atcab_read_serial_number(sn);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_read_serial_number is failed with status 0x%x\n",
                status);
        return 2;
    }

    status = atcab_nonce_rand(num_in, rand_out);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_nonce_rand is failed with status 0x%x\n",
                status);
        return 2;
    }

    struct atca_temp_key temp_key;
    memset(&temp_key, 0, sizeof (temp_key));

    struct atca_nonce_in_out nonce_params = {
        .mode = NONCE_MODE_SEED_UPDATE,
        .zero = 0,
        .num_in = num_in,
        .rand_out = rand_out,
        .temp_key = &temp_key,
    };
    status = atcah_nonce(&nonce_params);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcah_nonce is failed with status 0x%x\n", status);
        return 2;
    }

    struct atca_check_mac_in_out check_mac = {
        .mode = 1,
        .key_id = slot_id,
        .sn = sn,
        .client_chal = NULL,
        .client_resp = resp,
        .other_data = other_data,
        .otp = NULL,
        .slot_key = key,
        .target_key = NULL,
        .temp_key = &temp_key,
    };
    status = atcah_check_mac(&check_mac);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcah_check_mac is failed with status 0x%x\n", status);
        return 2;
    }

    /* check MAC */
    status = atcab_checkmac(1, slot_id, NULL, resp, other_data);
    if (status == ATCA_CHECKMAC_VERIFY_FAILED) {
        eprintf("Authentication failure\n");
        return 1;
    }
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_checkmac is failed with status 0x%x\n",
                status);
        return 2;
    }

    return 0;
}

void atecc_auth_passwd_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> password_file\n"
            "Authorizes key to use in next commands in row using password.\n"
            "\tslot_id\tID of slot with authorizing key.\n"
            "\tpassword_file\tInput stream with password (file or stdin).\n"
            "Password ends with EOF or newline. Max length of password is %d\n",
            cmdname, MAX_PASSWD_LEN);
}

int do_atecc_auth_make_passwd(int argc, char **argv)
{
    if (argc < 3) {
        atecc_auth_make_passwd_help(argv[0]);
        return 2;
    }

    uint16_t slot_id = atoi(argv[1]);
    const char *passwd_file = argv[2];

    /* read password */
    char passwd[MAX_PASSWD_LEN + 2];
    if (!read_passwd(passwd_file, passwd)) {
        return 2;
    }

    /* make a key from password */
    uint8_t key[ATCA_KEY_SIZE];
    if (!make_key_from_password(passwd, key)) {
        return 2;
    }

    /* write key unencrypted */
    ATCA_STATUS status;
    status = atcab_write_bytes_zone(ATCA_ZONE_DATA, slot_id, 0, key, ATCA_KEY_SIZE);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_write_bytes_zone is failed with status 0x%x\n",
                status);
        return 2;
    }

    return 0;
}

void atecc_auth_make_passwd_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> password_file\n"
            "Makes a key from password.\n"
            "\tslot_id\tID of slot to write a key.\n"
            "\tpassword_file\tInput stream with password (file or stdin).\n"
            "Password ends with EOF or newline. Max length of password is %d\n",
            cmdname, MAX_PASSWD_LEN);
}

int do_atecc_auth_check_gendig(int argc, char **argv)
{
    if (argc < 3) {
        atecc_auth_check_gendig_help(argv[0]);
        return 2;
    }

    uint16_t gendig_slot = atoi(argv[1]);
    const char *keyfilename = argv[2];
    uint8_t key[ATCA_KEY_SIZE];
    if (!read_key(keyfilename, key)) {
        return 2;
    }

    ATCA_STATUS status;
    atca_nonce_in_out_t nonce_params;
    atca_gen_dig_in_out_t gen_dig_param;
    atca_temp_key_t temp_key;
    uint8_t serial_num[32];
    uint8_t num_in[NONCE_NUMIN_SIZE] = { 0 };
    uint8_t rand_out[RANDOM_NUM_SIZE] = { 0 };
    uint8_t other_data[4] = { 0 };
    uint8_t resp[32];
    uint8_t challenge[32];

    if ((status = atcab_random(challenge)) != ATCA_SUCCESS)
    {
        eprintf("Command atcab_random is failed with status 0x%x\n", status);
        return 2;
    }


    // Read the device SN
    if ((status = atcab_read_zone(ATCA_ZONE_CONFIG, 0, 0, 0, serial_num, 32)) != ATCA_SUCCESS)
    {
        eprintf("Command atcab_read_zone is failed with status 0x%x\n", status);
        return 2;
    }
    // Make the SN continuous by moving SN[4:8] right after SN[0:3]
    memmove(&serial_num[4], &serial_num[8], 5);

    // Random Nonce inputs
    memset(&temp_key, 0, sizeof(temp_key));
    memset(&nonce_params, 0, sizeof(nonce_params));
    nonce_params.mode = NONCE_MODE_SEED_UPDATE;
    nonce_params.zero = 0;
    nonce_params.num_in = (uint8_t*)&num_in;
    nonce_params.rand_out = (uint8_t*)&rand_out;
    nonce_params.temp_key = &temp_key;

    // Send the random Nonce command
    if ((status = atcab_nonce_rand(num_in, rand_out)) != ATCA_SUCCESS)
    {
        eprintf("Command atcab_nonce_rand is failed with status 0x%x\n", status);
        return 2;
    }

    // Calculate Tempkey
    if ((status = atcah_nonce(&nonce_params)) != ATCA_SUCCESS)
    {
        eprintf("Command atcah_nonce is failed with status 0x%x\n", status);
        return 2;
    }

    // Supply OtherData so GenDig behavior is the same for keys with SlotConfig.NoMac set
    other_data[0] = ATCA_GENDIG;
    other_data[1] = GENDIG_ZONE_DATA;
    other_data[2] = (uint8_t)(gendig_slot);
    other_data[3] = (uint8_t)(gendig_slot >> 8);

    // Send the GenDig command
    if ((status = atcab_gendig(GENDIG_ZONE_DATA, gendig_slot, other_data, sizeof(other_data))) != ATCA_SUCCESS)
    {
        eprintf("Command atcab_gendig is failed with status 0x%x\n", status);
        return 2;
    }

    // Calculate Tempkey
    // NoMac bit isn't being considered here on purpose to remove having to read SlotConfig.
    // OtherData is built to get the same result regardless of the NoMac bit.
    memset(&gen_dig_param, 0, sizeof(gen_dig_param));
    gen_dig_param.key_id = gendig_slot;
    gen_dig_param.is_key_nomac = false;
    gen_dig_param.sn = serial_num;
    gen_dig_param.stored_value = key;
    gen_dig_param.zone = GENDIG_ZONE_DATA;
    gen_dig_param.other_data = other_data;
    gen_dig_param.temp_key = &temp_key;
    if ((status = atcah_gen_dig(&gen_dig_param)) != ATCA_SUCCESS)
    {
        eprintf("Command atcah_gen_dig is failed with status 0x%x\n", status);
        return 2;
    }

    // Now perform CheckMac to validate keys
    struct atca_check_mac_in_out check_mac = {
        .mode = 2,
        .key_id = 0, // TempKey used
        .sn = serial_num,
        .client_chal = challenge,
        .client_resp = resp,
        .other_data = other_data,
        .otp = NULL,
        .slot_key = key,
        .target_key = NULL,
        .temp_key = &temp_key,
    };
    status = atcah_check_mac(&check_mac);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcah_check_mac is failed with status 0x%x\n", status);
        return 2;
    }

    /* check MAC */
    status = atcab_checkmac(2, 0, challenge, resp, other_data);
    if (status == ATCA_CHECKMAC_VERIFY_FAILED) {
        eprintf("Keys doesn't match\n");
        return 1;
    }
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_checkmac is failed with status 0x%x\n",
                status);
        return 2;
    }

    eprintf("Keys match\n");
    return 0;
}

void atecc_auth_check_gendig_help(const char *cmdname)
{
    eprintf("Usage: %s <slot_id> key_file\n"
            "Checks key in selected slot matches key in file using GenDig.\n",
            cmdname);
}
