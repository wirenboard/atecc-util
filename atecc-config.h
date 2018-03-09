#pragma once

int do_atecc_dump_config(const char *args);
int do_atecc_read_config(const char *args);
int do_atecc_write_config(const char *args);
int do_atecc_lock_config(const char *args);

void atecc_dump_config_help(const char *cmdname);
void atecc_read_config_help(const char *cmdname);
void atecc_write_config_help(const char *cmdname);
void atecc_lock_config_help(const char *cmdname);
