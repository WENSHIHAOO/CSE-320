#include "reverki_utils.h"

/**
 * @brief Test function to test the rewriting of the term multiplicaton
 *
 */

Test(rewrite_suite, rewrite_multiplication_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    
    char *test_term = "(* (S (S 0)) (S ( S (S 0)))))";
    FILE *f = fmemopen(test_term, strlen(test_term), "r");
    REVERKI_TERM *ret_term = lib_reverki_parse_term(f);
    fclose(f);
    
   char *testt_term = "(+ (S (S 0)) (+ (S (S 0)) (+ (S (S 0)) (* (S (S 0)) 0))))";
    FILE *ff = fmemopen(testt_term, strlen(testt_term), "r");
    REVERKI_TERM *rett_term = reverki_parse_term(ff);
    fclose(ff);

    REVERKI_RULE *rule_list = NULL;
    REVERKI_RULE *rule;
    char *test_rule_in = "[(* x (S y)), (+ x (* x y))]";
    FILE *f_rule_in = fmemopen(test_rule_in, strlen(test_rule_in), "r");
    rule = lib_reverki_parse_rule(f_rule_in);
    rule->next = rule_list;
    rule_list = rule;

    // Test target
    REVERKI_TERM *ret = reverki_rewrite(rule_list,ret_term);
    assert_reverki_expected_term_struct(ret, rett_term);
    exit(COMMON_EXIT_CODE);
}
/**
 * @brief Test function to test the rewriting of the term addition
 *
 */

Test(rewrite_suite, rewrite_addition_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *test_term = "(S (+ (S (S 0)) (S (S 0))))";
    FILE *f = fmemopen(test_term, strlen(test_term), "r");
    REVERKI_TERM *ret_term = lib_reverki_parse_term(f);
    fclose(f);
     
    char *testt_term = "(S (S (S (+ (S (S 0)) 0))))";
    FILE *ff = fmemopen(testt_term, strlen(testt_term), "r");
    REVERKI_TERM *rett_term = reverki_parse_term(ff);
    fclose(ff);

    REVERKI_RULE *rule_list = NULL;
    REVERKI_RULE *rule;
    char *test_rule_in = "[(+ x (S y)), (S (+ x y))]";
    FILE *f_rule_in = fmemopen(test_rule_in, strlen(test_rule_in), "r");
    rule = lib_reverki_parse_rule(f_rule_in);
    rule->next = rule_list;
    rule_list = rule;

    // Test Target
    REVERKI_TERM *ret = reverki_rewrite(rule_list,ret_term);
    assert_reverki_expected_term_struct(ret, rett_term);
    exit(COMMON_EXIT_CODE);
}

