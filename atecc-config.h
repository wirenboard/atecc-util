#pragma once

int do_atecc_dump_config(int argc, char **argv);
int do_atecc_read_config(int argc, char **argv);
int do_atecc_write_config(int argc, char **argv);
int do_atecc_lock_config(int argc, char **argv);
int do_atecc_lock_slot(int argc, char **argv);
int do_atecc_config_is_locked(int argc, char **argv);
int do_atecc_extra_set(int argc, char **argv);
int do_atecc_extra_get(int argc, char **argv);

void atecc_dump_config_help(const char *cmdname);
void atecc_read_config_help(const char *cmdname);
void atecc_write_config_help(const char *cmdname);
void atecc_lock_config_help(const char *cmdname);
void atecc_lock_slot_help(const char *cmdname);
void atecc_config_is_locked_help(const char *cmdname);
void atecc_extra_set_help(const char *cmdname);
void atecc_extra_get_help(const char *cmdname);
