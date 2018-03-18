#pragma once

#include <stdio.h>
#include "basic/atca_basic.h"

ATCA_STATUS atecc_init(ATCAIfaceCfg *cfg);

int do_atecc_print_serial(int argc, char **argv);
