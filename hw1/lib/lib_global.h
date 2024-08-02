/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */
#ifndef LIB_GLOBAL_H
#define LIB_GLOBAL_H

#include <stdio.h>


long lib_global_options;

#define HELP_OPTION (0x00000001)
#define VALIDATE_OPTION (0x00000002)
#define REWRITE_OPTION (0x00000004)
#define TRACE_OPTION (0x00000008)
#define STATISTICS_OPTION (0x00000010)
#define LIMIT_OPTION (0x00000020)

/*
 * Buffer for accumulating the print name of an atom during parsing.
 * You *must* use this, because you are not allowed to declare any arrays.
 */
char lib_reverki_pname_buffer[REVERKI_PNAME_BUFFER_SIZE];

/*
 * Storage for atoms.  Every atom is stored in one of the structures in this
 * array.  An entry in the array is in use if and only if it has a non-zero
 * type field.  At any time, there can be at most one atom with a given pname.
 * We do this so that atoms can be compared using pointer equality, rather
 * than having to compare their pnames.
 */
#define LIB_REVERKI_NUM_ATOMS 100
REVERKI_ATOM lib_reverki_atom_storage[LIB_REVERKI_NUM_ATOMS];

/*
 * Atom-related functions that you are to implement.
 * See the stubs in atom.c for specifications.
 */
extern REVERKI_ATOM *lib_reverki_parse_atom(FILE *in);
extern int lib_reverki_unparse_atom(REVERKI_ATOM *atom, FILE *out);

/*
 * Storage for terms.  Every term is stored in one of the structures in this
 * array.  An entry in the array is in use if and only if it has a non-zero
 * type field.  It is permitted for two distinct entries in this array to
 * represent equal terms.  This means that to compare terms for equality,
 * we actually have to compare them using a recursive traversal.  Although we
 * could arrange that no two terms in the array are equal (as we do for atoms),
 * this makes things more complicated and we are not going to go there.
 */
#define LIB_REVERKI_NUM_TERMS 10000
REVERKI_TERM lib_reverki_term_storage[LIB_REVERKI_NUM_TERMS];

/*
 * Term-related functions that you are to implement.
 * See the stubs in term.c for specifications.
 */
extern REVERKI_TERM *lib_reverki_make_variable(REVERKI_ATOM *atom);
extern REVERKI_TERM *lib_reverki_make_constant(REVERKI_ATOM *atom);
extern REVERKI_TERM *lib_reverki_make_pair(REVERKI_TERM *fst, REVERKI_TERM *snd);
extern REVERKI_TERM *lib_reverki_parse_term(FILE *in);
extern int lib_reverki_unparse_term(REVERKI_TERM *term, FILE *out);
extern int lib_reverki_compare_term(REVERKI_TERM *term1, REVERKI_TERM *term2);

/*
 * Storage for rules.  Every rule is stored in one of the structures in this
 * array.  An entry in the array is in use if and only if its left-hand side
 * is non-NULL.
 */
#define LIB_REVERKI_NUM_RULES 1000
REVERKI_RULE lib_reverki_rule_storage[LIB_REVERKI_NUM_RULES];

/*
 * Rule-related functions that you are to implement.
 * See the stubs in rule.c for specifications.
 */
extern REVERKI_RULE *lib_reverki_make_rule(REVERKI_TERM *lhs, REVERKI_TERM *rhs);
extern REVERKI_RULE *lib_reverki_parse_rule(FILE *in);
extern int lib_reverki_unparse_rule(REVERKI_RULE *term, FILE *out);

/*
 * Substitution-related functions that you are to implement.
 * See the stubs in subst.c for specifications.
 */
extern int lib_reverki_match(REVERKI_TERM *pat, REVERKI_TERM *tgt, REVERKI_SUBST *substp);
extern REVERKI_TERM *lib_reverki_apply(REVERKI_SUBST subst, REVERKI_TERM *term);

/*
 * Function you are to implement that performs rewriting of a specified term,
 * given a list of rules.  See the stub in rewrite.c for specifications.
 */
extern REVERKI_TERM *lib_reverki_rewrite(REVERKI_RULE *rule_list, REVERKI_TERM *term);

/*
 * Function you are to implement that validates and interprets command-line arguments
 * to the program.  See the stub in validargs.c for specifications.
 */
extern int lib_validargs(int argc, char **argv);

#endif
