#include "atecc-init.h"

#include "util.h"
#include "helpers.h"

static int atecc_print_serial(FILE *output)
{
    ATCA_STATUS status;
    uint8_t sn[9];

    ATECC_RETRY(status, atcab_read_serial_number(sn));
    if(status != ATCA_SUCCESS) {
        printf("Command atcab_read_serial_number is failed with status %x\n", status);
        return 2;
    }

    fprintf(output, "%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", sn[0], sn[1], sn[2], sn[3], sn[4], sn[5], sn[6], sn[7], sn[8]);

    return 0;
}

ATCA_STATUS atecc_init(ATCAIfaceCfg *cfg)
{
    ATCA_STATUS status;

    ATECC_RETRY(status, atcab_init(cfg));
    if(status != ATCA_SUCCESS) {
        eprintf("Command atcab_init is failed with status %x\n", status);
        return status;
    }

    return status;
}

int do_atecc_print_serial(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    return atecc_print_serial(stdout);
}
