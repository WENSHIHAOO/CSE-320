#include <stdio.h>

#include <criterion/criterion.h>
#include <criterion/logging.h>

#include "test_common.h"


Test(basecode_suite, no_arguments) {
    char *name = "no_arguments";
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
}

Test(basecode_suite, nontrivial_path) {
    char *name = "nontrivial_path";
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/ empty.16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
}

Test(basecode_suite, asm_basic) {
    char *name = "asm_basic";
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/asm_basic/ moo.16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
    assert_binfile_matches("moo");
}

Test(basecode_suite, empty_input) {
    char *name = "empty_input";
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/empty_input/ empty.16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
    assert_binfile_matches("empty");
}

/*
 * Run the program with default options on a non-empty input file
 * and use valgrind to check for uninitialized values.
 */
Test(basecode_suite, valgrind_uninitialized) {
    char *name = "valgrind_uninitialized";
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/ moo.16");
    int err = run_using_system(name, "", "valgrind --leak-check=full --undef-value-errors=no --error-exitcode=37", STANDARD_LIMITS);
    assert_no_valgrind_errors(err);
    assert_expected_status(EXIT_SUCCESS, err);
}
