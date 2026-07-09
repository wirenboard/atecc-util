#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Forward declare the function under test from atecc-auth.c */
extern int atecc_auth_parse_serial(const char *input, char *output, size_t output_len);

START_TEST(test_buffer_read_bounds)
{
    /* Invariant: Buffer reads never exceed declared length; oversized inputs
       must be truncated or rejected without out-of-bounds access */
    
    const char *payloads[] = {
        "12345678901234567890",           /* Valid: 20 chars */
        "123456789012345678901234567890123456789012345678901234567890",  /* 2x oversized */
        "1",                              /* Boundary: minimal valid */
        "12345678901234567890123456789012345678901234567890123456789012345678901234567890"  /* 4x oversized */
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);
    
    for (int i = 0; i < num_payloads; i++) {
        char output[32];
        memset(output, 0xAA, sizeof(output));  /* Fill with sentinel */
        
        /* Call production function with bounded output buffer */
        int result = atecc_auth_parse_serial(payloads[i], output, 20);
        
        /* Verify: function either succeeds with truncation or fails gracefully */
        ck_assert(result >= 0 || result == -1);
        
        /* Verify: no write beyond declared output_len boundary */
        for (int j = 20; j < (int)sizeof(output); j++) {
            ck_assert_msg((unsigned char)output[j] == 0xAA,
                "Buffer overflow detected at offset %d for payload %d", j, i);
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_buffer_read_bounds);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}