#include <criterion/criterion.h>
#include <criterion/logging.h>

#include "reverki.h"
#include "global.h"
#include "reverki_utils.h"



Test(basecode_suite, help_system_test, .timeout=10) {
    char *cmd = "ulimit -t 5; bin/reverki -h < /dev/null > /dev/null 2>&1";

    // system is a syscall defined in stdlib.h
    // it takes a shell command as a string and runs it
    // we use WEXITSTATUS to get the return code from the run
    // use 'man 3 system' to find out more
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with 0x%x instead of EXIT_SUCCESS",
         return_code);
}

/**
 * @brief Test function for addition substitution
 *
 */

Test(blackbox_suite, reverki_blackbox_passtest_addition, .timeout=10) {
    char *cmd = "ulimit -t 5; bin/reverki -r < rsrc/addition > test_output/addition.out";
    //char *cmp = "cmp test_output/addition.out tests/rsrc/addition.out";

    int return_code = WEXITSTATUS(system(cmd));
    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with 0x%x instead of EXIT_SUCCESS",
		 return_code);
    return_code = black_box_compare_structure_using_file("tests/rsrc/addition.out", "test_output/addition.out");
    cr_assert_eq(return_code, 1,"Program output did not match reference output.");
}

/**
 * @brief Test function for algebra substitution
 *
 */

Test(blackbox_suite, reverki_blackbox_passtest_algebra, .timeout=10) {
    char *cmd = "ulimit -t 5; bin/reverki -r < rsrc/algebra > test_output/algebra.out";
    //char *cmp = "cmp test_output/algebra.out tests/rsrc/algebra.out";

    int return_code = WEXITSTATUS(system(cmd));
    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with 0x%x instead of EXIT_SUCCESS",
		 return_code);
    return_code = black_box_compare_structure_using_file("tests/rsrc/algebra.out", "test_output/algebra.out");
    cr_assert_eq(return_code, 1,
                 "Program output did not match reference output.");
}

/**
 * @brief Test function for combinators substitution
 *
 */

Test(blackbox_suite, reverki_blackbox_passtest_combinators, .timeout=10) {
    char *cmd = "ulimit -t 5; bin/reverki -r < rsrc/combinators > test_output/combinators.out";
    //char *cmp = "cmp test_output/combinators.out tests/rsrc/combinators.out";

    int return_code = WEXITSTATUS(system(cmd));
    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with 0x%x instead of EXIT_SUCCESS",
		 return_code);
    return_code = black_box_compare_structure_using_file("tests/rsrc/combinators.out", "test_output/combinators.out");
    cr_assert_eq(return_code, 1,
                 "Program output did not match reference output.");
}

/**
 * @brief Test function for multiplication substitution
 *
 */

Test(blackbox_suite, reverki_blackbox_passtest_multiplication, .timeout=10) {
    char *cmd = "ulimit -t 5; bin/reverki -r < rsrc/multiplication > test_output/multiplication.out";
    //char *cmp = "cmp test_output/multiplication.out tests/rsrc/multiplication.out";

    int return_code = WEXITSTATUS(system(cmd));
    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with 0x%x instead of EXIT_SUCCESS",
		 return_code);
    return_code = black_box_compare_structure_using_file("tests/rsrc/multiplication.out", "test_output/multiplication.out");
    cr_assert_eq(return_code, 1, "Program output did not match reference output.");
}

/**
 * @brief Test function for partial rule addition substitution
 *
 */

Test(blackbox_suite, reverki_blackbox_test_partial_addition, .timeout=10) {
    char *cmd = "ulimit -t 5; bin/reverki -r < rsrc/partial_addition > test_output/partial_addition.out";
    //char *cmp = "cmp test_output/partial_addition.out tests/rsrc/partial_addition.out";

    int return_code = WEXITSTATUS(system(cmd));
    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with 0x%x instead of EXIT_SUCCESS",
         return_code);
    return_code = black_box_compare_structure_using_file("tests/rsrc/partial_addition.out", "test_output/partial_addition.out");
    cr_assert_eq(return_code, 1,
                 "Program output did not match reference output.");
}

/**
 * @brief Test function for partial rule multiplication substitution
 *
 */

Test(blackbox_suite, reverki_blackbox_test_partial_multiplication, .timeout=10) {
    char *cmd = "ulimit -t 5; bin/reverki -r < rsrc/partial_multiplication > test_output/partial_multiplication.out";
    //char *cmp = "cmp test_output/partial_multiplication.out tests/rsrc/partial_multiplication.out";

    int return_code = WEXITSTATUS(system(cmd));
    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with 0x%x instead of EXIT_SUCCESS",
         return_code);
    return_code = black_box_compare_structure_using_file("tests/rsrc/partial_multiplication.out", "test_output/partial_multiplication.out");
    cr_assert_eq(return_code, 1,
                 "Program output did not match reference output.");
}





















