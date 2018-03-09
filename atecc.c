#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>

#include "basic/atca_basic.h"
#include "atecc_config_zone.h"

#include "config.h"
#include "helpers.h"

#include "atecc-init.h"
#include "atecc-config.h"

struct atecc_cmd {
    const char *name;
    int (*callback)(const char *);
    void (*help)(const char *);
};

struct atecc_cmd commands[] = {
    {"write-config", do_atecc_write_config, atecc_write_config_help},
    {"read-config", do_atecc_read_config, atecc_read_config_help},
    {"dump-config", do_atecc_dump_config, atecc_dump_config_help},
    {"lock-config", do_atecc_lock_config, atecc_lock_config_help},
    { NULL, NULL, NULL }
};

void print_available_cmds(void)
{
    struct atecc_cmd *cmds = commands;
    while (cmds->name != NULL) {
        eprintf("\t%s\n", cmds->name);
        cmds++;
    }
}

void print_help(const char *argv0, const char *cmd_name)
{
    if (cmd_name != NULL) {
        struct atecc_cmd *cmds = commands;
        while (cmds->name != NULL) {
            if (strcmp(cmds->name, cmd_name) == 0) {
                cmds->help(cmd_name);
                return;
            }
            cmds++;
        }
        eprintf("Unknown command: %s\nAvailable commands:\n", cmd_name);
        print_available_cmds();
    } else {
        eprintf("Usage: %s [-hbs] -c \"cmd1 cmd1_args\" [-c \"cmd2 cmd2_args\"]\n\n", argv0);

        eprintf("\t-b <i2c bus ID>\n\t\tI2C bus ID ATECC is connected to. Default is %d\n", DEFAULT_I2C_BUS);
        eprintf("\t-s <i2c slave ID>\n\t\tI2C slave ID of ATECC. Default is %d\n", DEFAULT_I2C_SLAVE);
        eprintf("\t-c \"cmd [arg1 [arg2 ...]]\"\n\t\tCommand and its arguments.\n");
        eprintf("\t-h[cmd_name]\n\t\tPrint this help message or help message of specific command.\n\n");

        eprintf("Available commands:\n");
        print_available_cmds();
    }
}

int main(int argc, char *argv[])
{
    int c, i;
    const char *argv0 = argv[0];
    char *cmds[MAX_CMDS];
    int num_cmds = 0;
    int ret = 0;

    ATCAIfaceCfg cfg = cfg_ateccx08a_i2c_default;
    cfg.atcai2c.bus = DEFAULT_I2C_BUS;
    cfg.atcai2c.slave_address = DEFAULT_I2C_SLAVE;

    while ((c = getopt(argc, argv, "h::c:b:s:")) != -1) {
        switch (c) {
        case 'h':
            print_help(argv0, optarg);
            return 0;
            break;
        case 'c':
            assert(num_cmds != MAX_CMDS);
            cmds[num_cmds++] = optarg;
            break;
        case 'b':
            cfg.atcai2c.bus = atoi(optarg);
            break;
        case 's':
            cfg.atcai2c.slave_address = atoi(optarg);
            break;
        default:
            eprintf("Unknown option: %c\n", c);
            print_help(argv0, NULL);
            exit(1);
        }
    }

    if (num_cmds == 0) {
        print_help(argv0, NULL);
        return 1;
    }

    /* init ATECC first */
    if (atecc_init(&cfg) != ATCA_SUCCESS) {
        exit(2);
    }

    for (i = 0; i < num_cmds; i++) {
        char *cmdname, *cmdargs;
        struct atecc_cmd *cmdlist = commands;

        cmdname = cmds[i];
        cmdargs = strchr(cmdname, ' ');

        if (cmdargs != NULL) {
            *cmdargs++ = '\0';
        } else {
            cmdargs = "";
        }

        while (cmdlist->name != NULL) {
            if (strcmp(cmdlist->name, cmdname) == 0) {
                ret = cmdlist->callback(cmdargs);
                if (ret != 0) {
                    goto _exit;
                }
                break;
            }
            cmdlist++;
        }
    }

_exit:
    atcab_release();

    return ret;
}
