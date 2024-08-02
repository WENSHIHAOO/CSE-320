#include "reverki_utils.h"

/**
 * @brief Test if arguments have been specified with '-'
 *
 */
Test(validargs_suite, opt_no_dash_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "h", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    assert_func_expected_status(ret, exp_ret, "validargs");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test if arguments given after -h are ignored and function returns success
 *
 */
Test(validargs_suite, ignore_args_after_help_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "-h", "-ignore_me", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int exp_opt = HELP_OPTION;
    assert_func_expected_status(ret, exp_ret, "validargs");
    assert_reverki_expected_options(opt, exp_opt);
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test if -v is set when asked to Validate
 *
 */
Test(validargs_suite, validate_only_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "-v", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int exp_opt = VALIDATE_OPTION;
    assert_func_expected_status(ret, exp_ret, "validargs");
    assert_reverki_expected_options(opt, exp_opt);
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test if -r is set when asked to Rewrite
 *
 */
Test(validargs_suite, rewrite_only_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "-r", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int exp_opt = REWRITE_OPTION;
    assert_func_expected_status(ret, exp_ret, "validargs");
    assert_reverki_expected_options(opt, exp_opt);
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Options -v and -r should not be specified together
 *
 */
Test(validargs_suite, rewrite_n_validate_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "-v", "-r", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    assert_func_expected_status(ret, exp_ret, "validargs");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Options -v and -t should not be specified together
 *
 */
Test(validargs_suite, validate_n_trace_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "-v", "-t", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    assert_func_expected_status(ret, exp_ret, "validargs");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Option -l cannot be specified standalone.
 *
 */
Test(validargs_suite, only_l_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "-l", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    assert_func_expected_status(ret, exp_ret, "validargs");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Option -t cannot be specified standalone.
 *
 */
Test(validargs_suite, only_t_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "-t", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    assert_func_expected_status(ret, exp_ret, "validargs");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test if non-integer argument passed with -l is validated
 *
 */
Test(validargs_suite, invalid_lim_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "-r", "-l", "str", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    assert_func_expected_status(ret, exp_ret, "validargs");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test if limit is specified within limit 2^31-1
 *
 */
Test(validargs_suite, exceed_lim_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "-r", "-l", "3000000000", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    assert_func_expected_status(ret, exp_ret, "validargs");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Limit should not be specified before the rewrite option
 *
 */
Test(validargs_suite, limit_b4_rewrite_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "-l", "1000", "-r", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    assert_func_expected_status(ret, exp_ret, "validargs");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test if limit is stored correctly
 *
 */
Test(validargs_suite, limit_store_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *argv[] = {progname, "-r", "-l", "255", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    long opt = global_options;
    long exp_opt = REWRITE_OPTION | LIMIT_OPTION | (255L << 32);
    assert_func_expected_status(ret, exp_ret, "validargs");
    assert_reverki_expected_options(opt, exp_opt);
    exit(COMMON_EXIT_CODE);
}
