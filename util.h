#pragma once

#include <stdint.h>
#include "basic/atca_basic.h"

int sha256_file(const char *filename, uint8_t *output);

/**
 * Checks if it is OK to retry operation.
 * Uses static counter for this.
 */
int should_retry(ATCA_STATUS status);

/**
 * Resets retry counter to specific value.
 * Used systemwise for each command
 */
void retry_counter_reset(int value);

/**
 * A macro to put ATECC function call in
 * retry loop
 */
#define ATECC_RETRY(status, call) do { \
        status = call; \
    } while (should_retry(status))
