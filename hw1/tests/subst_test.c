#include "reverki_utils.h"

Test(subst_suite, correct_reverki_match, .timeout = 10, .exit_code = COMMON_EXIT_CODE) {
	REVERKI_RULE *rule_list = NULL;
	REVERKI_RULE *rule;
	char *test_rule_in = "[(+ x (S y)), (S (+ x y))]";
    FILE *f_rule_in = fmemopen(test_rule_in, strlen(test_rule_in), "r");
    rule = reverki_parse_rule(f_rule_in);
    rule->next = rule_list;
	rule_list = rule;
	REVERKI_TERM *term;
	char *test_term_in = "(+ (S (S 0)) (S (S (S 0))))";
	FILE *f_term_in = fmemopen(test_term_in, strlen(test_term_in), "r");
	term = reverki_parse_term(f_term_in);

	fclose(f_rule_in);
	fclose(f_term_in);
 	REVERKI_SUBST subst = NULL;
	int ret = reverki_match(rule->lhs, term, &subst);
	int exp = 1;

	assert_func_expected_status(ret, exp, "correct_reverki_match");
    exit(COMMON_EXIT_CODE);
}

Test(subst_suite, incorrect_reverki_match, .timeout = 10, .exit_code = COMMON_EXIT_CODE) {
	REVERKI_RULE *rule_list = NULL;
	REVERKI_RULE *rule;
	char *test_rule_in = "[(+ x 0), x]";
    FILE *f_rule_in = fmemopen(test_rule_in, strlen(test_rule_in), "r");
    rule = reverki_parse_rule(f_rule_in);
    rule->next = rule_list;
	rule_list = rule;
	REVERKI_TERM *term;
	char *test_term_in = "(+ (S (0 0)) (S (S (S 0))))";
	FILE *f_term_in = fmemopen(test_term_in, strlen(test_term_in), "r");
	term = reverki_parse_term(f_term_in);

	fclose(f_rule_in);
	fclose(f_term_in);
 	REVERKI_SUBST subst = NULL;
	int ret = reverki_match(rule->lhs, term, &subst);
	int exp = 0;

	assert_func_expected_status(ret, exp, "incorrect_reverki_match");
    exit(COMMON_EXIT_CODE);
}

Test(subst_suite, invalid_variable_type_test_in_reverki_apply, .timeout = 10, .exit_code = COMMON_EXIT_CODE) {
     REVERKI_TERM in_term = {
        .type = REVERKI_NO_TYPE
    };
	REVERKI_SUBST subst = NULL;
    int exp = 1;
    int ret = 0;
    REVERKI_TERM *ret_term;

    ret_term = reverki_apply(subst, &in_term);
    if (ret_term == NULL || in_term.type == ret_term->type|| ret_term == (REVERKI_TERM *)-1)
    	ret = 1;

    assert_func_expected_status(ret, exp, "reverki_apply_No_type");
    exit(COMMON_EXIT_CODE);
}

Test(subst_suite, valid_constant_type_test_in_reverki_apply, .timeout = 10, .exit_code = COMMON_EXIT_CODE) {
     REVERKI_TERM in_term = {
        .type = REVERKI_CONSTANT_TYPE
    };
	REVERKI_SUBST subst = NULL;
    int exp = 1;
    int ret = 0;
    REVERKI_TERM *ret_term;
    ret_term = reverki_apply(subst, &in_term);
    if (!compare_reverki_terms(&in_term, ret_term))
    	ret = 1;

    assert_func_expected_status(ret, exp, "reverki_apply_Constant_type");

    exit(COMMON_EXIT_CODE);
}









