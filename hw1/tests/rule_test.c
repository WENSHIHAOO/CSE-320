#include "reverki_utils.h"

/**
 * @brief Test function if input stream contains left bracket
 *
 */
Test(rule_suite, reverki_parse_rule_test_withoutleftbracket, .timeout = 10, .exit_code = COMMON_EXIT_CODE)
{
    char *test_rule = "(+ x 0), \n[(+ x (S y)), (S (+ x y))]\n(+ (S (S 0)) (S (S (S 0))))";
    FILE *f = fmemopen(test_rule, strlen(test_rule), "r");
    REVERKI_RULE *ret_rule = reverki_parse_rule(f);
    assert_reverki_expected_null_ptr(ret_rule, "reverki_parse_rule");
    fclose(f);
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function if input stream contains right bracket
 *
 */
Test(rule_suite, reverki_parse_rule_test_withoutrightbracket, .timeout = 10, .exit_code = COMMON_EXIT_CODE)
{
    char *test_rule = "(+ x 0), x]\n[(+ x (S y)), (S (+ x y))]\n(+ (S (S 0)) (S (S (S 0))))";
    FILE *f = fmemopen(test_rule, strlen(test_rule), "r");
    REVERKI_RULE *ret_rule = reverki_parse_rule(f);
    assert_reverki_expected_null_ptr(ret_rule, "reverki_parse_rule");
    fclose(f);
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function if input stream contains left hand side rule
 *
 */

Test(rule_suite, reverki_parse_rule_test_withoutlhs, .timeout = 10, .exit_code = COMMON_EXIT_CODE)
{
    char *test_rule = "[, x]\n[(+ x (S y)), (S (+ x y))]\n(+ (S (S 0)) (S (S (S 0))))";
    FILE *f = fmemopen(test_rule, strlen(test_rule), "r");
    REVERKI_RULE *ret_rule = reverki_parse_rule(f);
    assert_reverki_expected_null_ptr(ret_rule, "reverki_parse_rule");
    fclose(f);
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function if input stream contains right hand side rule
 *
 */

Test(rule_suite, reverki_parse_rule_test_withoutrhs, .timeout = 10, .exit_code = COMMON_EXIT_CODE)
{
    char *test_rule = "[(+ x 0), ]\n[(+ x (S y)), (S (+ x y))]\n(+ (S (S 0)) (S (S (S 0))))";
    FILE *f = fmemopen(test_rule, strlen(test_rule), "r");
    REVERKI_RULE *ret_rule = reverki_parse_rule(f);
    assert_reverki_expected_null_ptr(ret_rule, "reverki_parse_rule");
    fclose(f);
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function if input stream contains comma
 *
 */
Test(rule_suite, reverki_parse_rule_test_withoutcomma, .timeout = 10, .exit_code = COMMON_EXIT_CODE)
{
    char *test_rule = "[(+ x 0) x]\n[(+ x (S y))  (S (+ x y))]\n(+ (S (S 0)) (S (S (S 0))))";
    FILE *f = fmemopen(test_rule, strlen(test_rule), "r");
    REVERKI_RULE *ret_rule = reverki_parse_rule(f);
    assert_reverki_expected_null_ptr(ret_rule, "reverki_parse_rule");
    fclose(f);
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function if input is in a correct format
 *
 */
Test(rule_suite, reverki_parse_rule_test_in_correct_format, .timeout = 10, .exit_code = COMMON_EXIT_CODE)
{
    char *test_rule_in = "[(+ x (S y)), (S (+ x y))]";
    FILE *f_in = fmemopen(test_rule_in, strlen(test_rule_in), "r");

    FILE *f_out = fopen("test_output/single_rule.out", "w+");

    REVERKI_RULE *ret_rule = reverki_parse_rule(f_in);
    reverki_unparse_rule(ret_rule, f_out);
    fseek(f_in, 0, SEEK_SET);
    fseek(f_out, 0, SEEK_SET);

    char *input_stripped = strip_string(readFile(f_in));
    char *output_stripped = strip_string(readFile(f_out));

    assert_reverki_expected_rule_match_string(input_stripped , output_stripped);

    fclose(f_in);
    fclose(f_out);

    exit(COMMON_EXIT_CODE);
}


