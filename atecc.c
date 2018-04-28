#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <wordexp.h>

#include "basic/atca_basic.h"
#include "atecc_config_zone.h"

#include "config.h"
#include "helpers.h"
#include "util.h"

#include "atecc-init.h"
#include "atecc-config.h"
#include "atecc-hmac.h"
#include "atecc-asymm.h"
#include "atecc-data.h"
#include "atecc-counter.h"
#include "atecc-ecdh.h"
#include "atecc-auth.h"

#ifndef VERSION
#define VERSION "(unknown version)"
#endif

#ifdef USE_OPENSSL
#define WITH_OPENSSL ", with OpenSSL"
#else
#define WITH_OPENSSL ""
#endif

#define DEFAULT_RETRIES 3

struct atecc_cmd {
    const char *name;
    int (*callback)(int, char**);
    void (*help)(const char *);
};

struct atecc_cmd commands[] = {
    {"serial", do_atecc_print_serial, NULL},
    {"write-config", do_atecc_write_config, atecc_write_config_help},
    {"read-config", do_atecc_read_config, atecc_read_config_help},
    {"dump-config", do_atecc_dump_config, atecc_dump_config_help},
    {"lock-config", do_atecc_lock_config, atecc_lock_config_help},
    {"config-is-locked", do_atecc_config_is_locked, atecc_config_is_locked_help},
    {"hmac-write-key", do_atecc_hmac_write_key, atecc_hmac_write_key_help},
    {"hmac-dgst", do_atecc_hmac_dgst, atecc_hmac_dgst_help},
    {"ecc-gen", do_atecc_gen_private, atecc_gen_private_help},
    {"ecc-write", do_atecc_write_private, atecc_write_private_help},
    {"ecc-read-pub", do_atecc_read_pub, atecc_read_pub_help},
    {"ecc-gen-pub", do_atecc_gen_pub, atecc_gen_pub_help},
    {"ecc-sign", do_atecc_sign, atecc_sign_help},
    {"ecc-verify", do_atecc_verify, atecc_verify_help},
    {"write-data", do_atecc_write_data, atecc_write_data_help},
    {"write-data-block", do_atecc_hmac_write_key, atecc_hmac_write_key_help},
    {"read-data", do_atecc_read_data, atecc_read_data_help},
    {"lock-data", do_atecc_lock_data, atecc_lock_data_help},
    {"data-is-locked", do_atecc_data_is_locked, atecc_data_is_locked_help},
    {"lock-slot", do_atecc_lock_slot, atecc_lock_slot_help},
    {"slot-is-locked", do_atecc_slot_is_locked, atecc_slot_is_locked_help},
    {"counter-read", do_atecc_read_counter, atecc_read_counter_help},
    {"counter-inc", do_atecc_increase_counter, atecc_increase_counter_help},
    {"counter-init", do_atecc_init_counter, atecc_init_counter_help},
    {"extra-set", do_atecc_extra_set, atecc_extra_set_help},
    {"extra-get", do_atecc_extra_get, atecc_extra_get_help},
    {"ecdh", do_atecc_ecdh, atecc_ecdh_help},
    {"auth-passwd", do_atecc_auth_passwd, atecc_auth_passwd_help},
    {"auth-make-passwd", do_atecc_auth_make_passwd, atecc_auth_make_passwd_help},
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

void print_version(void)
{
    eprintf("atecc-util " VERSION WITH_OPENSSL ", build " __DATE__ " " __TIME__ "\n");
}

void print_help(const char *argv0, const char *cmd_name)
{
    if (cmd_name != NULL) {
        struct atecc_cmd *cmds = commands;
        while (cmds->name != NULL) {
            if (strcmp(cmds->name, cmd_name) == 0) {
                if (cmds->help) {
                    cmds->help(cmd_name);
                } else {
                    eprintf("There's no help for command %s\n", cmds->name);
                }
                return;
            }
            cmds++;
        }
        eprintf("Unknown command: %s\nAvailable commands:\n", cmd_name);
        print_available_cmds();
    } else {
        print_version();
        eprintf("Usage: %s [-bshv] -c \"cmd1 cmd1_args\" [-c \"cmd2 cmd2_args\"]\n\n", argv0);

        eprintf("\t-b <i2c bus ID>\n\t\tI2C bus ID ATECC is connected to. Default is %d\n", DEFAULT_I2C_BUS);
        eprintf("\t-s <i2c slave ID>\n\t\tI2C slave ID of ATECC. Default is 0x%x\n", DEFAULT_I2C_SLAVE);
        eprintf("\t-c \"cmd [arg1 [arg2 ...]]\"\n\t\tCommand and its arguments.\n");
        eprintf("\t-h[cmd_name]\n\t\tPrint this help message or help message of specific command.\n");
        eprintf("\t-r <num_retries>\n\t\tMax number of retries for some commands. Default is %d\n", DEFAULT_RETRIES);
        eprintf("\t-v\tPrint version and exit\n\n");

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
    int num_retries = DEFAULT_RETRIES;

    ATCAIfaceCfg cfg = cfg_ateccx08a_i2c_default;
    cfg.atcai2c.bus = DEFAULT_I2C_BUS;
    cfg.atcai2c.slave_address = DEFAULT_I2C_SLAVE;

    while ((c = getopt(argc, argv, "h::c:b:s:vr:")) != -1) {
        switch (c) {
        case 'v':
            print_version();
            return 0;
            break;
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
        case 'r':
            num_retries = atoi(optarg);
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
        wordexp_t p;

        if (wordexp(cmds[i], &p, 0)) {
            eprintf("cmd '%s': commandline parse error\n", cmds[i]);
            ret = 1;
            goto _exit;
        }

        struct atecc_cmd *cmdlist = commands;
        while (cmdlist->name != NULL) {
            if (!p.we_wordv[0]) {
                print_help(argv0, NULL);
                ret = 1;
                wordfree(&p);
                goto _exit;
            }

            /* reset retry counter for each new command */
            retry_counter_reset(num_retries);

            if (strcmp(cmdlist->name, p.we_wordv[0]) == 0) {
                ret = cmdlist->callback(p.we_wordc, p.we_wordv);
                if (ret != 0) {
                    goto _exit;
                }
                break;
            }
            cmdlist++;
        }

        // if we are here, no such command found
        if (cmdlist->name == NULL) {
            eprintf("Unknown command: %s\n", p.we_wordv[0]);
            print_help(argv0, NULL);
            wordfree(&p);
            ret = 1;
            goto _exit;
        }

        wordfree(&p);
    }

_exit:
    atcab_release();

    return ret;
}
