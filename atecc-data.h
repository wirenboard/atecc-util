#pragma once

int do_atecc_write_data(int argc, char **argv);
void atecc_write_data_help(const char *cmdname);

int do_atecc_read_data(int argc, char **argv);
void atecc_read_data_help(const char *cmdname);

int do_atecc_lock_data(int argc, char **argv);
void atecc_lock_data_help(const char *cmdname);

int do_atecc_read_counter(int argc, char **argv);
void atecc_read_counter_help(const char *cmdname);
