#pragma once

#include <stdint.h>
#include "basic/atca_basic.h"

/**
 * SHA256 function is calculated either on ATECC or using OpenSSL
 * if it is selected in configuration
 */
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
