#include "test_common.h"

/**
 * @brief Test function for testing env variable in a good condition
 *
 */

#define NAME basic_pass_test1_env
#define INPUT dsktst
Test(basic_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:bin/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    int return_code = access("test_output/" QUOTE(NAME) "/" QUOTE(INPUT) ".bin", F_OK);
    cr_assert_eq(return_code, 0, "The program did not produce binary output as it should have done");
}
#undef NAME
#undef INPUT

#define NAME options_prefix
#define INPUT dsktst
Test(options_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:bin/");
    sprintf(program_options, "%s", "--output-dir test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    int return_code = access("test_output/" QUOTE(NAME) "/" QUOTE(INPUT) ".bin", F_OK);
    cr_assert_eq(return_code, 0, "The program did not produce binary output as it should have done");
}
#undef NAME
#undef INPUT

#if 0  // This produces results basically redundant with env_fail_notExistFile
/**
 * @brief Test function for testing env variable with an unintended delimiter
 *
 */

#define NAME env_fail_comma_separated
#define INPUT dsktst
Test(env_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/,bar/,tests/rsrc/,bin/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    char *cmd = "grep -q \"Can't open\" test_output/" QUOTE(NAME) ".err";
    int ret_grep =  WEXITSTATUS(system(cmd));
    cr_assert_eq(ret_grep, 0, "The program did not report that it couldn't open the file");
    int ret_make_output = count_files_in_a_dir("test_output/" QUOTE(NAME) "/");
    cr_assert_eq(ret_make_output, 0, "The program produced output files when it shouldn't have");
}
#undef NAME
#undef INPUT
#endif

/**
 * @brief Test function for testing env variable when input file does not exist
 *
 */

#define NAME env_fail_notExistFile
#define INPUT dsktst128
Test(env_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:bin/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    char *cmd = "grep -q \"Can't open\" test_output/" QUOTE(NAME) ".err";
    int ret_grep =  WEXITSTATUS(system(cmd));
    cr_assert_eq(ret_grep, 0, "The program did not report that it couldn't open the file");
    int ret_make_output = count_files_in_a_dir("test_output/" QUOTE(NAME) "/");
    cr_assert_eq(ret_make_output, 0, "The program produced output files when it shouldn't have");
}
#undef NAME
#undef INPUT

/**
 * @brief Test function for testing env variable when a directory name's len is 60 char
 *
 */

#define NAME env_pass_longpath
#define INPUT dsktst100
Test(env_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:tests/rsrc/qwertyuiopasdfghjklzxcvbnm1234567890qazxswedcvfrtgb12345678/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    int ret_make_output = count_files_in_a_dir("test_output/" QUOTE(NAME) "/");
    cr_assert(ret_make_output, "The program did not produce output when it should have");
}
#undef NAME
#undef INPUT

/**
 * @brief Test function for testing env variable path name Buffer Overflow
 *
 */

#define NAME env_pass_longpath_BoF
#define INPUT dsktst101
Test(env_suite, NAME, .timeout=10)
 {
   char *name = QUOTE(NAME);
   putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:tests/rsrc/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA/");
   sprintf(program_options, "%s", QUOTE(INPUT) ".16 -O test_output/" QUOTE(NAME) "/");
   int err = run_using_system(name, "", "", STANDARD_LIMITS);
   assert_expected_status(EXIT_SUCCESS, err);
   char *cmd = "grep -q \"Can't open file\" test_output/" QUOTE(NAME) ".err";
   int ret_grep =  WEXITSTATUS(system(cmd));
   cr_assert_eq(ret_grep, 1, "The program reported that it couldn't open the file");
   int ret_make_output = count_files_in_a_dir("test_output/" QUOTE(NAME) "/");
   cr_assert(ret_make_output, "The program did not produce output files when it should have");
}
#undef NAME
#undef INPUT

/**

 * @brief Test if arguments given before -h are ignored and function returns success
 *
 */

#define NAME options_ignore_args_after_help_short
#define INPUT dsktst
Test(options_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:bin/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16 -h");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    char *cmd_out = "grep -q --ignore-case -e usage -e no-second-pass test_output/" QUOTE(NAME) ".out";
    int ret_grep_out = WEXITSTATUS(system(cmd_out));
    char *cmd_err = "grep -q --ignore-case -e usage -e no-second-pass test_output/" QUOTE(NAME) ".err";
    int ret_grep_err = WEXITSTATUS(system(cmd_err));
    // Allow the usage message either on stdout or stderr, because the specs apparently did not require stderr.
    cr_assert((!ret_grep_out) || (!ret_grep_err), "The program did not print a usage message");
    int ret_make_output = count_files_in_a_dir("test_output/" QUOTE(NAME) "/");
    cr_assert_eq(ret_make_output, 0, "The program produced output files when it shouldn't have");
}
#undef NAME
#undef INPUT

/**

 * @brief Test if arguments given before -h are ignored and function returns success
 *
 */

#define NAME options_ignore_args_after_help_long
#define INPUT dsktst
Test(options_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:bin/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16 --help");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    char *cmd_out = "grep -q --ignore-case -e usage -e no-second-pass test_output/" QUOTE(NAME) ".out";
    int ret_grep_out = WEXITSTATUS(system(cmd_out));
    char *cmd_err = "grep -q --ignore-case -e usage -e no-second-pass test_output/" QUOTE(NAME) ".err";
    int ret_grep_err = WEXITSTATUS(system(cmd_err));
    // Allow the usage message either on stdout or stderr, because the specs apparently did not require stderr.
    cr_assert((!ret_grep_out) || (!ret_grep_err), "The program did not print a usage message");
    int ret_make_output = count_files_in_a_dir("test_output/" QUOTE(NAME) "/");
    cr_assert_eq(ret_make_output, 0, "The program produced output files when it shouldn't have");
}
#undef NAME
#undef INPUT


/**
 * @brief Test if included file not exist
 *
 */

#define NAME options_not_exist_include
#define INPUT dsktst_not_exist_include
Test(options_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:bin/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    char *cmd = "grep \"Can't include\" test_output/" QUOTE(NAME) ".err";
    fprintf(stderr, "RUNNING: %s\n", cmd);
    int ret_grep = WEXITSTATUS(system(cmd));
    fprintf(stderr, "%s (%d)\n", "test_output/" QUOTE(NAME) ".err", ret_grep);
    cr_assert_eq(ret_grep, 0, "The program did not report that it couldn't include the file");
    int ret_make_output = count_files_in_a_dir("test_output/" QUOTE(NAME) "/");
    cr_assert_eq(ret_make_output, 0, "The program produced output files when it shouldn't have");
}
#undef NAME
#undef INPUT

/**
 * @brief Test if included file is in a newPath
 *
 */

#define NAME options_incld_in_newPath
#define INPUT dsktst_incld_newPath
Test(options_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:tests/rsrc/newPath/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    int ret_make_output = count_files_in_a_dir("test_output/" QUOTE(NAME) "/");
    cr_assert(ret_make_output, "The program did not produce output files when it should have");
}
#undef NAME
#undef INPUT


/**
 * @brief Test if output file name extension
 *
 */

#define NAME options_check_output_filename
#define INPUT dsktst
Test(options_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:bin/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_normal_exit(err);
    int return_code = access("test_output/" QUOTE(NAME) "/" QUOTE(INPUT) ".bin", F_OK);
    cr_assert_eq(return_code, 0, "The program did not produce a .bin file as it should have done");
}
#undef NAME
#undef INPUT

/**
 * @brief Test if --no-second-option (longform) is implemented correctly (Positive Test)
 * @details Positive Test: Test with a source file which does not contain any errors.
 * Tests for: 1. Program is expected to exit successfully
 *            2. No output on standard output
 *            3. No Binary produced
 */

#define NAME options_no_second_pass
#define INPUT dsktst
Test(options_suite, NAME) {
    fprintf(stdout, "Starting Tests..............\n");
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:bin/");
    sprintf(program_options, "%s", "--no-second-pass -O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);

    // Check that no binary file has been produced
    int bin_file_exists = (access("test_output/" QUOTE(NAME) "/" QUOTE(INPUT) ".bin", F_OK) == 0);
    cr_assert_eq(bin_file_exists, 0, "Program produced output binary when it was not supposed to!");

    // Check that no stdout was produced by the file
    assert_outfile_matches(name, NULL);
}
#undef NAME
#undef INPUT

/**
 * @brief Test if --no-second-option (longform) is implemented correctly (Negative Test)
 * @details Negative Test: Test with a source file which contains errors.
 * Tests for: 1. Program is expected to exit with EXIT_FAILURE
 *            2. No Output on Standard Output
 *            3. Output on Standard Error
 *            4. No Binary produced
 */

#define NAME options_no_second_pass_neg
#define INPUT dsktst_has_err
Test(options_suite, NAME) {
    fprintf(stdout, "Starting Tests..............\n");
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:bin/");
    sprintf(program_options, "%s", "--no-second-pass -O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);

    // Program should exit with EXIT_FAILURE
    assert_expected_status(EXIT_FAILURE, err);

    // Check that no binary file has been produced
    int bin_file_exists = (access("test_output/" QUOTE(NAME) "/" QUOTE(INPUT) ".bin", F_OK) == 0);
    cr_assert_eq(bin_file_exists, 0, "Program produced output binary when it was not supposed to!");

    // Check that no stdout was produced by the file
    assert_outfile_matches(name, NULL);

    // Check that some error reporting was done on stderr
    assert_errfile_matches(name, "Undefined Symbols");
}
#undef NAME
#undef INPUT

/**
 * @brief Test if -2 (shortform of --no-second-pass) is implemented correctly (Positive Test)
 * @details Positive Test: Test with a source file which does not contain any errors.
 * Tests for: 1. Program is expected to exit successfully
 *            2. No output on standard output
 *            3. No Binary produced
 * Will carry out only one test for shortform as longform tests must cover other  scenarios
 */

#define NAME options_no_second_pass_shortform
#define INPUT dsktst
Test(options_suite, NAME) {
    fprintf(stdout, "Starting Tests..............\n");
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=foo/:bar/:tests/rsrc/:bin/");
    sprintf(program_options, "%s", "-2 -O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);

    assert_expected_status(EXIT_SUCCESS, err);

    // Check that no binary file has been produced
    int bin_file_exists = (access("test_output/" QUOTE(NAME) "/" QUOTE(INPUT) ".bin", F_OK) == 0);
    cr_assert_eq(bin_file_exists, 0, "Program produced output binary when it was not supposed to!");

    // Check that no stdout was produced by the file
    assert_outfile_matches(name, NULL);
}
#undef NAME
#undef INPUT

/**
 * @brief Test if output binary matches - memtest
 *
 */

#define NAME asm_basic_memtst
#define INPUT memtst
Test(basic_suite, NAME) {
  char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
    assert_binfile_matches(QUOTE(INPUT));
}
#undef NAME
#undef INPUT

/**
 * @brief Test if output binary matches - intr
 *
 */

#define NAME asm_basic_intr
#define INPUT intr
Test(basic_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
    assert_binfile_matches(QUOTE(INPUT));
}
#undef NAME
#undef INPUT

/**
 * @brief Test if output binary matches - print
 *
 */

#define NAME asm_basic_print
#define INPUT print
Test(basic_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
    assert_binfile_matches(QUOTE(INPUT));
}
#undef NAME
#undef INPUT

#if 0  // Results are pretty much redundant with asm_basic_print
/**
 * @brief Test if output binary matches - type
 *
 */

#define NAME asm_basic_type
#define INPUT type
Test(basic_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
    assert_binfile_matches(QUOTE(INPUT));
}
#undef NAME
#undef INPUT
#endif

#if 0  // Results are pretty much redundant with other tests in this suite
/**
 * @brief Test if output binary matches - vdemo
 *
 */

#define NAME asm_basic_vdemo
#define INPUT vdemo
Test(basic_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
    assert_binfile_matches(QUOTE(INPUT));
}
#undef NAME
#undef INPUT
#endif

#if 0  // Results are pretty much redundant with other tests in this suite
/**
 * @brief Test if output binary matches - vt52
 *
 */

#define NAME asm_basic_vt52
#define INPUT vt52
Test(basic_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
    assert_binfile_matches(QUOTE(INPUT));
}
#undef NAME
#undef INPUT

/**
 * @brief Test if both output and Binary file matches for omon (Stress Test)
 *
 */

#define NAME asm_basic_omon
#define INPUT omon
Test(basic_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
    assert_binfile_matches(QUOTE(INPUT));
}
#undef NAME
#undef INPUT
#endif

/**
 * @brief Test defs.16, which should just produce a symbol table and no code or data.
 */

#define NAME asm_extra_defs
#define INPUT defs
Test(extra_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
}
#undef NAME
#undef INPUT

/**
 * @brief Test a short code snippet that should fail if "int obuf" has not been fixed.
 */

#define NAME obuf_bug
#define INPUT obuf_bug
Test(extra_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_binfile_matches(QUOTE(INPUT));
}
#undef NAME
#undef INPUT

/**
 * @brief Test a short code snippet that should fail if the branch offset bug has not been fixed.
 */

#define NAME branch_offset_bug
#define INPUT branch_offset_bug
Test(extra_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_binfile_matches(QUOTE(INPUT));
}
#undef NAME
#undef INPUT

/**
 * @brief Test a short code snippet that should fail if the error("g") bug has not been fixed.
 */

#define NAME garbage_bug
#define INPUT garbage_bug
Test(extra_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_FAILURE, err);
    char *cmd = "fgrep -q 'garbage_bug.16:1: *** g' test_output/" QUOTE(NAME) ".err";
    int ret_grep =  WEXITSTATUS(system(cmd));
    cr_assert_eq(ret_grep, 0, "The program did not report the garbage character error");
}
#undef NAME
#undef INPUT

/**
 * @brief Test a short code snippet that should fail if null character output bug has not been fixed.
 */

#define NAME null_char_bug
#define INPUT null_char_bug
Test(extra_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
}
#undef NAME
#undef INPUT

/**
 * @brief Test a short code snippet that should fail if the sign bit error in printout has not been fixed.
 */

#define NAME sign_bit_bug
#define INPUT sign_bit_bug
Test(extra_suite, NAME) {
    char *name = QUOTE(NAME);
    putenv("ASM_INPUT_PATH=tests/rsrc/");
    sprintf(program_options, "%s", "-O test_output/" QUOTE(NAME) "/ " QUOTE(INPUT) ".16");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
}
#undef NAME
#undef INPUT

