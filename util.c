#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "helpers.h"

#if !defined(USE_ATECC_SHA256) && !defined(USE_OPENSSL_SHA256)
#warning "No SHA256 calculation method defined, use ATECC by default"
#define USE_ATECC_SHA256
#endif

#if defined(USE_OPENSSL_SHA256)
#include <openssl/sha.h>
#elif defined(USE_ATECC_SHA256)
#include "basic/atca_basic.h"
#endif

#define BUFFER_SIZE 256

int sha256_file(const char *filename, uint8_t *output)
{
    uint8_t buffer[BUFFER_SIZE];
    FILE *input = maybe_fopen(filename, "rb");
    if (!input) {
        perror("open message file for reading");
        return 1;
    }

#if defined(USE_OPENSSL_SHA256)
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
#elif defined(USE_ATECC_SHA256)
    ATCA_STATUS status;
    atca_sha256_ctx_t sha256;
    status = atcab_hw_sha2_256_init(&sha256);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_hw_sha2_256_init is failed with status 0x%x\n", status);
        maybe_fclose(input);
        return 2;
    }
#endif

    while (!feof(input)) {
        size_t bytesRead = fread(buffer, 1, sizeof (buffer), input);
        if (bytesRead == 0 && !feof(input)) {
            perror("read shunk from message file");
            maybe_fclose(input);
            return 1;
        }

#if defined(USE_OPENSSL_SHA256)
        SHA256_Update(&sha256, buffer, bytesRead);
#elif defined(USE_ATECC_SHA256)
        status = atcab_hw_sha2_256_update(&sha256, buffer, bytesRead);
        if (status != ATCA_SUCCESS) {
            eprintf("Command atcab_hw_sha2_256_update is failed with status 0x%x\n", status);
            maybe_fclose(input);
            return 2;
        }
#endif
    }

#if defined(USE_OPENSSL_SHA256)
    SHA256_Final(output, &sha256);
#elif defined(USE_ATECC_SHA256)
    status = atcab_hw_sha2_256_finish(&sha256, output);
    if (status != ATCA_SUCCESS) {
        eprintf("Command atcab_hw_sha2_256_finish is failed with status 0x%x\n", status);
        maybe_fclose(input);
        return 2;
    }
#endif

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
        case ATCA_GEN_FAIL:
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
