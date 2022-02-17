#pragma once

#include <stdio.h>
#include "cryptoauthlib.h"

ATCA_STATUS atecc_init(ATCAIfaceCfg *cfg);

int do_atecc_print_serial(int argc, char **argv);
int do_atecc_print_info(int argc, char **argv);
