#include "reverki_utils.h"

/**
 * @brief Test behaviour in absence of any arguments
 *
 */
Test(validargs_suite, no_args_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    assert_func_expected_status(ret, exp_ret, "basecode");
    exit(COMMON_EXIT_CODE);
}



