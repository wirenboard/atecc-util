#pragma once

#define HMAC_KEY_SIZE 32
#define PAYLOAD_BUFFER_SIZE 128

int do_atecc_hmac_write_key(int argc, char **argv);
void atecc_hmac_write_key_help(const char *cmdname);

int do_atecc_hmac_dgst(int argc, char **argv);
void atecc_hmac_dgst_help(const char *cmdname);
