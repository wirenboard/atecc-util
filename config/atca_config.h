/* Auto-generated config file atca_config.h */
#ifndef ATCA_CONFIG_H
#define ATCA_CONFIG_H

#define ATCA_HAL_I2C

/* Included device support */
#define ATCA_ATECC508A_SUPPORT
#define ATCA_ATECC608_SUPPORT

/** Device Override - Library Assumes ATECC608B support in checks */
#define ATCA_ATECC608A_SUPPORT

/* \brief How long to wait after an initial wake failure for the POST to
 *         complete.
 * If Power-on self test (POST) is enabled, the self test will run on waking
 * from sleep or during power-on, which delays the wake reply.
 */
#ifndef ATCA_POST_DELAY_MSEC
#define ATCA_POST_DELAY_MSEC 25
#endif

#define ATCA_USE_CONSTANT_HOST_NONCE

/***************** Diagnostic & Test Configuration Section *****************/

/** Enable debug messages */
#define ATCA_PRINTF

/** Enable to build in test hooks */
/* #cmakedefine ATCA_TESTS_ENABLED */

/******************** Features Configuration Section ***********************/

/** Define to enable older API forms that have been replaced */
#define ATCA_ENABLE_DEPRECATED

/******************** Platform Configuration Section ***********************/

#define ATCA_PLATFORM_MALLOC    malloc
#define ATCA_PLATFORM_FREE      free

#define atca_delay_ms   hal_delay_ms
#define atca_delay_us   hal_delay_us

#endif // ATCA_CONFIG_H
