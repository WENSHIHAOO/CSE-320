#include <stdio.h>

#include <reverki.h>

extern int lib_reverki_trace;

/*
 * Utility functions: not specified in the assignment, but
 * students might choose to introduce these.
 */
extern int lib_reverki_is_whitespace(int c);
extern int lib_reverki_is_letter(int c);
extern int lib_reverki_is_uppercase_letter(int c);
extern int lib_reverki_strcmp(char *str1, char *str2);
extern void lib_reverki_strcpy(char *dst, char *src);
extern int lib_reverki_skip_whitespace(FILE *in);

void lib_reverki_show_atoms(FILE *out);
extern void lib_reverki_show_terms(FILE *out);
extern void lib_reverki_show_subst(REVERKI_SUBST subst, FILE *out);
