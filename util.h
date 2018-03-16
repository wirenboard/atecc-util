#pragma once

#include <stdint.h>

/**
 * SHA256 function is calculated either on ATECC or using OpenSSL
 * if it is selected in configuration
 */
int sha256_file(const char *filename, uint8_t *output);
