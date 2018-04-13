#pragma once

int do_atecc_gen_private(int argc, char **argv);
void atecc_gen_private_help(const char *cmdname);

int do_atecc_write_private(int argc, char **argv);
void atecc_write_private_help(const char *cmdname);

int do_atecc_read_pub(int argc, char **argv);
void atecc_read_pub_help(const char *cmdname);

int do_atecc_gen_pub(int argc, char **argv);
void atecc_gen_pub_help(const char *cmdname);

int do_atecc_sign(int argc, char **argv);
void atecc_sign_help(const char *cmdname);

int do_atecc_verify(int argc, char **argv);
void atecc_verify_help(const char *cmdname);
