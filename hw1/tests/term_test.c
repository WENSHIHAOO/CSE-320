#include "reverki_utils.h"

/**
 * @brief Test function which creates variables with names having more than one character
 *
 */
Test(term_suite, mk_multi_char_variable_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    REVERKI_ATOM test_atom = {
        .type = REVERKI_VARIABLE_TYPE,
        .pname = {'v','a','r'},
        .next = NULL
    };

    REVERKI_TERM exp_term = {
        .type = REVERKI_VARIABLE_TYPE,
        .value = {
            .atom = &test_atom
        },
    };

    REVERKI_TERM *ret = reverki_make_variable(&test_atom);

    assert_reverki_expected_term_struct(ret, &exp_term);
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function which creates constant with names having more than one character
 *
 */
Test(term_suite, mk_multi_char_const_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    REVERKI_ATOM test_atom = {
        .type = REVERKI_CONSTANT_TYPE,
        .pname = {'A','b','C'},
        .next = NULL
    };

    REVERKI_TERM exp_term = {
        .type = REVERKI_CONSTANT_TYPE,
        .value = {
            .atom = &test_atom
        },
    };

    REVERKI_TERM *ret = reverki_make_constant(&test_atom);

    assert_reverki_expected_term_struct(ret, &exp_term);
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function which creates Pairs out of Terms
 *
 */
Test(term_suite, mk_pair_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    REVERKI_ATOM test_atom1 = {
        .type = REVERKI_CONSTANT_TYPE,
        .pname = {'+'},
        .next = NULL
    };

    REVERKI_ATOM test_atom2 = {
        .type = REVERKI_VARIABLE_TYPE,
        .pname = {'a','b','C'},
        .next = NULL
    };

    REVERKI_TERM test_term1 = {
        .type = REVERKI_CONSTANT_TYPE,
        .value = {
            .atom = &test_atom1
        },
    };

    REVERKI_TERM test_term2 = {
        .type = REVERKI_VARIABLE_TYPE,
        .value = {
            .atom = &test_atom2
        },
    };

    REVERKI_TERM exp_pair = {
        .type = REVERKI_PAIR_TYPE,
        .value = {
            .pair = {
                .fst = &test_term1,
                .snd = &test_term2
            }
        },
    };

    REVERKI_TERM *ret = reverki_make_pair(&test_term1, &test_term2);

    assert_reverki_expected_term_struct(ret, &exp_pair);
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Compare for equality of two equal terms
 * @details Each term has one atom which is a variable
 *
 */
Test(term_suite, equal_terms_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    REVERKI_ATOM test_atom1 = {
        .type = REVERKI_VARIABLE_TYPE,
        .pname = {'a','b','c'},
        .next = NULL
    };

    REVERKI_TERM test_term1 = {
        .type = REVERKI_VARIABLE_TYPE,
        .value = {
            .atom = &test_atom1
        },
    };

    REVERKI_TERM test_term2 = {
        .type = REVERKI_VARIABLE_TYPE,
        .value = {
            .atom = &test_atom1 // Since code is expected to have unique atoms
        },
    };

    int exp = 0;
    int ret = reverki_compare_term(&test_term1, &test_term2);

    assert_func_expected_status(ret, exp, "reverki_compare_term");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Negative Test - Compare for equality of two unequal terms
 *
 */
Test(term_suite, unequal_terms_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    REVERKI_ATOM test_atom1 = {
        .type = REVERKI_CONSTANT_TYPE,
        .pname = {'+'},
        .next = NULL
    };

    REVERKI_ATOM test_atom2 = {
        .type = REVERKI_VARIABLE_TYPE,
        .pname = {'a','b','C'},
        .next = NULL
    };

    REVERKI_TERM test_term1 = {
        .type = REVERKI_CONSTANT_TYPE,
        .value = {
            .atom = &test_atom1
        },
    };

    REVERKI_TERM test_term2 = {
        .type = REVERKI_VARIABLE_TYPE,
        .value = {
            .atom = &test_atom2
        },
    };

    int exp = 0;
    int ret = reverki_compare_term(&test_term1, &test_term2);
    assert_func_unexpected_status(ret, exp, "reverki_compare_term");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test improperly matched paranthesis
 *
 */
Test(term_suite, paran_mismatch_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *test_term = "(S (S (S (S (S 0))";
    FILE *f = fmemopen(test_term, strlen(test_term), "r");
    REVERKI_TERM *ret_term = reverki_parse_term(f);
    fclose(f);
    assert_reverki_expected_null_ptr(ret_term, "reverki_parse_term");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test if able to process arbitrary ' ' and \t in between the term
 * @details Should return not NULL
 * (!!! How to verify that the pointer returned is a valid parsed term and not some not null garbage?)
 * Not checking for newlines as students might be using newlines to differentiate between terms on different lines
 *
 */
Test(term_suite, whitespace_ignore_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *test_term = "   (S (S (S (S (S     \t 0)))))";
    FILE *f = fmemopen(test_term, strlen(test_term), "r");
    REVERKI_TERM *ret_term = reverki_parse_term(f);
    fclose(f);
    assert_reverki_expected_notnull_ptr(ret_term, "reverki_parse_term");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test if returns null on passing only paranthesis
 *
 */
Test(term_suite, only_paran_no_term_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *test_term = "((()))";
    FILE *f = fmemopen(test_term, strlen(test_term), "r");
    REVERKI_TERM *ret_term = reverki_parse_term(f);
    fclose(f);
    assert_reverki_expected_null_ptr(ret_term, "reverki_parse_term");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test to leave stray commas
 *
 */
Test(term_suite, stray_comma_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
    char *test_term = "(S (S (S (S , , (S, 0)))))";
    FILE *f = fmemopen(test_term, strlen(test_term), "r");
    REVERKI_TERM *ret_term = reverki_parse_term(f);
    fclose(f);
    assert_reverki_expected_null_ptr(ret_term, "reverki_parse_term");
    exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test to verify if unparse_term prints the term correctly
 *
 */
Test(term_suite, unparse_term_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {

    // Create term
    char *test_term_str = "((f (DIFF (g x))) + (g (DIFF (f x))))";
    FILE *f = fmemopen(test_term_str, strlen(test_term_str), "r");
    REVERKI_TERM *test_term = lib_reverki_parse_term(f);
    fclose(f);

    char ret_term[128] = {0}; // Assuming students can return a larger string with more paranthesis
    f = fmemopen(ret_term, 128, "w");

    int exp = 0;
    // Target test function
    int u_ret = reverki_unparse_term(test_term, f);
    fclose(f);

    // Check if they return non zero for a supposed succesful parse
    assert_func_expected_status(u_ret, exp, "reverki_unparse_term");

    f = fmemopen(ret_term, 128, "r");

    // Check if returned string is a valid term that can be parsed
    REVERKI_TERM *ret_unparsed_term = lib_reverki_parse_term(f);
    fclose(f);

    int ret = compare_reverki_terms(ret_unparsed_term, test_term);

    if(ret)
        cr_log_error("Incorrect unparsing of Term. Passed Term: %s; Got Term: %s", \
                                                                test_term_str, ret_term);
    assert_func_expected_status(ret, exp, "reverki_strcmp_util");

    exit(COMMON_EXIT_CODE);
}