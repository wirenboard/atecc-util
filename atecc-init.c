#include "atecc-init.h"

#include "helpers.h"

ATCA_STATUS atecc_init(ATCAIfaceCfg *cfg)
{
    ATCA_STATUS status;

    status = atcab_init(cfg);
    if(status != ATCA_SUCCESS) {
        eprintf("Command atcab_init is failed with status %x\n", status);
        return status;
    }

    uint8_t sn[9];
    status = atcab_read_serial_number(sn);
    if(status != ATCA_SUCCESS) {
        printf("Command atcab_read_serial_number is failed with status %x\n", status);
        return status;
    }

    eprintf("Serial: %02X %02X %02X %02X %02X %02X %02X %02X %02X \n", sn[0], sn[1], sn[2], sn[3], sn[4], sn[5], sn[6], sn[7], sn[8]);

    return status;
}
