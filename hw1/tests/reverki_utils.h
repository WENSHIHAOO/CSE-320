#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "reverki.h"
#include "global.h"
#include "lib_global.h"
#include "string.h"

static char *progname = "bin/reverki";

#define COMMON_EXIT_CODE 101

/**
 * @brief Strip whitespaces and parantheses from the string
 *
 */
static int reverki_strip_whitespace_n_paran(char *stream, char *new_str) {

    int idx = 0;
    for(int i = 0; i <= strlen(stream)-1; i++)
    {
        char c = stream[i];
        if(c == '(' || c == ' ' || c == ')')
            continue;
        new_str[idx++] = c;
    }
    return 0;
}

/**
 * @brief Utility function to compare two strings
 *
 */
static int reverki_strcmp_util(char *str1, char *str2) {
    return strcmp(str1, str2);
}

/**
 * @brief Deep Compare two atoms
 * @details This function will be invoked when pointers to atoms do not match
 * However there might be a chance that the function has returned a pointer to
 * program's internal atoms storage in which case even after correctly storing the atom,
 * expected and returned pointers will differ
 *
 */

static int deep_cmp_atoms(REVERKI_ATOM* atom1, REVERKI_ATOM* atom2)
{
    if(atom1->type != atom2->type)
        return 1;

    if(reverki_strcmp_util(atom1->pname, atom2->pname))
        return 1;

    // Not comparing atom1->next and atom2->next

    return 0;
}

/**
 * @brief Compare two structs of type REVERKI_TERM
 *
 */
static int compare_reverki_terms(REVERKI_TERM *term1, REVERKI_TERM *term2)
{
    if(term1 == NULL) {
      cr_log_error("Attempting to compare NULL with a term\n");
      return 1;
    }
    if(term1 == NULL) {
      cr_log_error("Attempting to compare a term with NULL\n");
      return 1;
    }
    if(term1 == term2)
    {
        return 0;
    }

    if(term1->type != term2->type)
    {
        cr_log_error("Term Type Mismatch. Returned Term Type: %d, Expected Term Type: %d", \
                                        term1->type, term2->type);
        return 1;
    }

    switch(term1->type)
    {
        case REVERKI_VARIABLE_TYPE:
        case REVERKI_CONSTANT_TYPE:
            if(term1->value.atom != term2->value.atom)
            {
                // In case of pointer mismatch, perform deep compare
                if(deep_cmp_atoms(term1->value.atom, term2->value.atom))
                {
                    cr_log_error("Atoms Mismatch");
                    return 1;
                }
            }
	    break;
        case REVERKI_PAIR_TYPE:
            return(compare_reverki_terms(term1->value.pair.fst, term2->value.pair.fst) ||
            compare_reverki_terms(term1->value.pair.snd, term2->value.pair.snd));
        default:
            cr_log_error("Invalid Term Type %d", term1->type);
            return 1;
    }

    return 0;

}

/**
 * @brief compare two rule file
 *
 */
static int compare_reverki_rule_files(FILE *fp1, FILE *fp2)
{
    fseek(fp1,0,SEEK_SET);
    fseek(fp2,0,SEEK_SET);
    char ch1 = getc(fp1);
    char ch2 = getc(fp2);

    int pos = 0, line = 1;
    while (ch1 != EOF && ch2 != EOF)
    {
        pos++;
        if (ch1 == '\n' && ch2 == '\n')
        {
            line++;
            pos = 0;
        }
        if (ch1 != ch2)
        {
            return 0;
        }

        ch1 = getc(fp1);
        ch2 = getc(fp2);
    }
    return 1;
}

/**
 * @brief read a file and  store in a char *
 *
 */

static char* readFile(FILE *file)
{
    char *code;
    size_t n = 0;
    int c;

    if (file == NULL)
        return NULL; //could not open file

    code = malloc(100);

    while ((c = fgetc(file)) != EOF)
    {
        code[n++] = (char) c;
    }

    code[n] = '\0';

    return code;
}


/**
 * @brief remove white spaces from a given string
 *
 */

static char* strip_string(char* input)
{
    int loop;
    char *output = (char*) malloc (strlen(input));
    char *dest = output;

    if (output)
    {
        for (loop=0; loop<strlen(input); loop++)
            if (input[loop] != ' ' && input[loop] != '(' && input[loop] != ')')
                *dest++ = input[loop];

        *dest = '\0';
    }
    return output;
}

/**
 * @brief  Compare striped strings of output file and expected output file in the blackbox test
 *
 */
static int black_box_compare_structure_using_file(char *exp_output_addr, char *output_addr)
{
    FILE *exp_output = fopen(exp_output_addr, "r");
    FILE *output = fopen(output_addr, "r");

    REVERKI_TERM *ret = lib_reverki_parse_term(output);
    char ret_term[128] = {0};
    FILE *f = fmemopen(ret_term, 128, "w");

    lib_reverki_unparse_term(ret, f);
    int ret_value  = compare_reverki_rule_files(f,exp_output);

    fclose(exp_output);
    fclose(output);
    fclose(f);

    return ret_value;
}


static int black_box_compare_structure(char *exp_output_addr, char *output_addr)
{
    FILE *exp_output = fopen(exp_output_addr, "r");
    FILE *output = fopen(output_addr, "r");

    REVERKI_TERM *exp = lib_reverki_parse_term(exp_output);
    REVERKI_TERM *ret = lib_reverki_parse_term(output);

    int ret_value  = compare_reverki_terms(ret, exp);

    fclose(exp_output);
    fclose(output);

    return ret_value;
}
/**
 * @brief  Compare striped strings of output file and expected output file in the blackbox test
 *
 */
static int black_box_compare(char *exp_output_addr, char *output_addr)
{
    FILE *exp_output = fopen(exp_output_addr, "r");
    FILE *output = fopen(output_addr, "r");

    char *exp_output_stripped = strip_string(readFile(exp_output));
    char *output_stripped  = strip_string(readFile(output));

    fclose(exp_output);
    fclose(output);

    int ret_value = !reverki_strcmp_util(exp_output_stripped, output_stripped);
    return ret_value;
}
/**
 * @brief Assert return status for any function
 *
 */
static void assert_func_expected_status(int status, int expected, const char *caller)
{
	cr_assert_eq(status, expected,
			"Invalid return for %s. Got: %d | Expected: %d",
			caller, status, expected);
}

/**
 * @brief Assert return status for any function - Inequality
 *
 */
static void assert_func_unexpected_status(int status, int expected, const char *caller)
{
	cr_assert_neq(status, expected,
			"Invalid return for %s. Return should not be %d | Got: %d",
			caller, expected, status);
}

/**
 * @brief Assert expected value for global opts
 *
 */
static void assert_reverki_expected_options(int option, int expected)
{
    cr_assert_eq(option, expected, "Invalid options settings. Got: 0x%x | Expected: 0x%x",
			option, expected);
}

/**
 * @brief Assert expected struct REVERKI_TERM
 *
 */
static void assert_reverki_expected_term_struct(REVERKI_TERM *ret, REVERKI_TERM *exp)
{
    cr_assert_eq(compare_reverki_terms(ret, exp), 0,\
                     "Reverki Term Structure Mismatch.");
}

/**
 * @brief Assert expected pointer is NULL
 *
 */
static void assert_reverki_expected_null_ptr(void *ptr, const char* caller)
{
    cr_assert_null(ptr, "Test Failed: %s returned a non-NULL pointer, Expected NULL", caller);
}

/**
 * @brief Assert expected pointer is not NULL
 *
 */
static void assert_reverki_expected_notnull_ptr(void *ptr, const char* caller)
{
    cr_assert_not_null(ptr, "Test Failed: %s returned NULL pointer, Expected not-NULL", caller);
}

/**
 * @brief Assert expected struct REVERKI_RULE
 *
 */
static void assert_reverki_expected_rule_match(FILE *file1, FILE *file2)
{

    cr_assert_eq(compare_reverki_rule_files(file1, file2), 1,"Reverki Rule Matched");
}

/**
 * @brief Assert expected struct REVERKI_RULE
 *
 */
static void assert_reverki_expected_rule_match_string(char *in, char *out)
{
    cr_assert_eq(!reverki_strcmp_util(in, out), 1,"Reverki Rule Matched");
}


static void assert_normal_exit(int status)
{
    cr_assert_eq(status, EXIT_SUCCESS,
                 "Program exited with 0x%x instead of EXIT_SUCCESS",
				 status);
}

static void assert_outfile_matches(int status)
{
    cr_assert_eq(status, EXIT_SUCCESS,
                 "Program output did not match reference output.");
}

static void assert_string_matches_stripped(char *in, char *out)
{
    cr_assert_eq(!reverki_strcmp_util(in, out), 1,"Program output did not match reference output.");
}



