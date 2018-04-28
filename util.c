#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "helpers.h"

#include "crypto/atca_crypto_sw.h"

#define BUFFER_SIZE 256

int sha256_file(const char *filename, uint8_t *output)
{
    ATCA_STATUS status;
    uint8_t buffer[BUFFER_SIZE];
    FILE *input = maybe_fopen(filename, "rb");
    if (!input) {
        perror("open message file for reading");
        return 1;
    }

    atcac_sha2_256_ctx ctx;
    status = atcac_sw_sha2_256_init(&ctx);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcac_sw_sha2_256_init is failed with status 0x%x\n", status);
        maybe_fclose(input);
        return 2;
    }

    while (!feof(input)) {
        size_t bytesRead = fread(buffer, 1, sizeof (buffer), input);
        if (bytesRead == 0 && !feof(input)) {
            perror("read shunk from message file");
            maybe_fclose(input);
            return 1;
        }

        status = atcac_sw_sha2_256_update(&ctx, buffer, bytesRead);
        if (status != ATCA_SUCCESS) {
            eprintf("Command atcac_sw_sha2_256_update is failed with status 0x%x\n", status);
            maybe_fclose(input);
            return 2;
        }
    }

    status = atcac_sw_sha2_256_finish(&ctx, output);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcac_sw_sha2_256_finish is failed with status 0x%x\n", status);
        maybe_fclose(input);
        return 2;
    }

    maybe_fclose(input);
    return 0;
}

static int __retry_counter;

static int error_safe(ATCA_STATUS status)
{
    switch (status)
    {
        case ATCA_CONFIG_ZONE_LOCKED:
        case ATCA_DATA_ZONE_LOCKED:
        case ATCA_CHECKMAC_VERIFY_FAILED:
        case ATCA_PARSE_ERROR:
        case ATCA_FUNC_FAIL:
        case ATCA_BAD_PARAM:
        case ATCA_INVALID_ID:
        case ATCA_INVALID_SIZE:
        case ATCA_TOO_MANY_COMM_RETRIES:
        case ATCA_SMALL_BUFFER:
        case ATCA_BAD_OPCODE:
        case ATCA_WAKE_SUCCESS:
        case ATCA_EXECUTION_ERROR:
        case ATCA_UNIMPLEMENTED:
        case ATCA_ASSERT_FAILURE:
        case ATCA_NOT_LOCKED:
        case ATCA_NO_DEVICES:
            return 0;
        default:
            return 1;
    }
}

int should_retry(ATCA_STATUS status)
{
    if (error_safe(status) && __retry_counter > 0) {
        __retry_counter--;
        eprintf("Attempts left: %d\n", __retry_counter + 1);
        return 1;
    } else {
        return 0;
    }
}

void retry_counter_reset(int value)
{
    __retry_counter = value;
}
