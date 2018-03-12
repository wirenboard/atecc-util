#pragma once

int do_atecc_dump_config(int argc, char **argv);
int do_atecc_read_config(int argc, char **argv);
int do_atecc_write_config(int argc, char **argv);
int do_atecc_lock_config(int argc, char **argv);

void atecc_dump_config_help(const char *cmdname);
void atecc_read_config_help(const char *cmdname);
void atecc_write_config_help(const char *cmdname);
void atecc_lock_config_help(const char *cmdname);
