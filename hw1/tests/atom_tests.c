#include "reverki_utils.h"

/**
 * @brief Test function which creates atom  with a constant
 *
 */
Test(atom_suite, pa_parse_atom_constant_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {

   char *test_atom = "S";
   FILE *f = fmemopen(test_atom, strlen(test_atom), "r");
   REVERKI_ATOM *atom_ret = reverki_parse_atom(f);
   fclose(f);
   assert_reverki_expected_notnull_ptr(atom_ret, "reverki_parse_atom");
   exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function which creates atom  with a variable
 *
 */
Test(atom_suite, pa_parse_atom_variable_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
   char *test_atom = "y";
   FILE *f = fmemopen(test_atom, strlen(test_atom), "r");
   REVERKI_ATOM *atom_ret = reverki_parse_atom(f);
   fclose(f);
   assert_reverki_expected_notnull_ptr(atom_ret, "reverki_parse_atom");
   exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function which creates atom  with a closing brackets
 *
 */
Test(atom_suite, pa_parse_atom_cbracket_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
   char *test_atom = "]";
   FILE *f = fmemopen(test_atom, strlen(test_atom), "r");
   REVERKI_ATOM *atom_ret = reverki_parse_atom(f);
   fclose(f);
   assert_reverki_expected_null_ptr(atom_ret, "reverki_parse_atom");
   exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function which creates atom  with a opening parenthesis
 *
 */
Test(atom_suite, pa_parse_atom_oparenthesis_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
   char *test_atom = "(";
   FILE *f = fmemopen(test_atom, strlen(test_atom), "r");
   REVERKI_ATOM *atom_ret = reverki_parse_atom(f);
   fclose(f);
   assert_reverki_expected_null_ptr(atom_ret, "reverki_parse_atom");
   exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function which creates atom  with a closing parenthesis
 *
 */
Test(atom_suite, pa_parse_atom_cparenthesis_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
   char *test_atom = ")";
   FILE *f = fmemopen(test_atom, strlen(test_atom), "r");
   REVERKI_ATOM *atom_ret = reverki_parse_atom(f);
   fclose(f);
   assert_reverki_expected_null_ptr(atom_ret, "reverki_parse_atom");
   exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function which creates atom  with a comma
 *
 */
Test(atom_suite, pa_parse_atom_comma_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
   char *test_atom = ",";
   FILE *f = fmemopen(test_atom, strlen(test_atom), "r");
   REVERKI_ATOM *atom_ret = reverki_parse_atom(f);
   fclose(f);
   assert_reverki_expected_null_ptr(atom_ret, "reverki_parse_atom");
   exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function which creates atom pname with max buffer size
 *
 */
Test(atom_suite, pa_parse_atom_maxbuffer_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
   int BUFF_XS = 128;
   char *test_atom = calloc(BUFF_XS,sizeof(char)+1);
   for(int i = 0; i < BUFF_XS; i++) test_atom[i] = 'a'; // Atom with pname 'aaaa..a' BUFF_XS times

   FILE *f = fmemopen(test_atom, strlen(test_atom), "r");
   REVERKI_ATOM *atom_ret = reverki_parse_atom(f);
   fclose(f);
   assert_reverki_expected_null_ptr(atom_ret, "reverki_parse_atom");
   exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function unparse atom for the following condition:
 * The pname of the specified atom is output to the specified output stream.
 * If the output is unsuccessful, then EOF is returned.
 *
 */
 
Test(atom_suite, pa_unparse_atom_for_EOF_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
   REVERKI_ATOM test_atom = {
        .type = REVERKI_VARIABLE_TYPE,
        .pname = {'t', 'o', 'o', ' ', 'l', 'o', 'n', 'g'},
        .next = NULL
    };
   FILE *f = fmemopen(NULL,1,"w");
   reverki_unparse_atom(&test_atom,f);
   int ret = fclose(f);
   int exp = 0;
   assert_func_expected_status(ret, EOF, "reverki_unparse_atom");
   exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test function unparse atom for the following condition:
 * The pname of the specified atom is output to the specified output stream.
 * If the output is successful, then 0 is returned.
 *
 */
Test(atom_suite, pa_unparse_atom_for_zero_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {
REVERKI_ATOM test_atom = {
        .type = REVERKI_VARIABLE_TYPE,
        .pname = {"s"},
        .next = NULL
    };
   //declaring the varible test_mem to write allocate the memory to write the atom
   int BUFF_XS = 128;
   char *test_mem = (char*)calloc(BUFF_XS,sizeof(char));
   for(int i = 0; i < (sizeof(char)*BUFF_XS); i++) test_mem[i] = 'a'; // Atom with pname 'aaaa..a' BUFF_XS times
   FILE *f = fmemopen(test_mem,sizeof(test_mem), "w");
   reverki_unparse_atom(&test_atom,f);
   int ret = fclose(f);
   int exp = 0;
   assert_func_expected_status(ret, exp, "reverki_unparse_atom");
   //Test if the unparse atom was the same as test_atom
   int rett = reverki_strcmp_util(test_atom.pname,test_mem);
    if(ret)
        cr_log_error("Incorrect unparsing of Atom.");
    assert_func_expected_status(rett, exp, "reverki_strcmp_util");

   exit(COMMON_EXIT_CODE);
}

/**
 * @brief Test for Unique Atoms - Create same atoms and compare the pointers
 *
 */
Test(atom_suite, pa_unique_atoms_test, .timeout = 5, .exit_code = COMMON_EXIT_CODE) {

   char *test_atom = "test_atom";
   FILE *f1 = fmemopen(test_atom, strlen(test_atom), "r");
   FILE *f2 = fmemopen(test_atom, strlen(test_atom), "r");
   REVERKI_ATOM *atom_ret1 = reverki_parse_atom(f1);
   REVERKI_ATOM *atom_ret2 = reverki_parse_atom(f2);
   fclose(f1);
   fclose(f2);

   int exp = 1;
   int ret = (atom_ret1 == atom_ret2);

   assert_func_expected_status(ret, exp, "unique_atoms_test");
   exit(COMMON_EXIT_CODE);
}
