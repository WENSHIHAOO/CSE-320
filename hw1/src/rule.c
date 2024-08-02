#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "reverki.h"
#include "global.h"

/*
 * @brief  Create a rule with a specified left-hand side and right-hand side terms.
 * @details  A rule is created that contains specified terms as its left-hand side
 * and right-hand side.
 * @param lhs  Term to use as the left-hand side of the rule.
 * @param rhs  Term to use as the right-hand side of the rule.
 * @return  A pointer to the newly created rule.
 */
REVERKI_RULE *reverki_make_rule(REVERKI_TERM *lhs, REVERKI_TERM *rhs) {
    struct reverki_rule *rule = reverki_rule_storage;
    int d=0;
    while(rule->next != NULL){
        d++;
        rule = reverki_rule_storage+d;
    }

    rule->next = reverki_rule_storage+d+1;

    rule->lhs = lhs;
    rule->rhs = rhs;
    return rule;
}

/*
 * @brief  Parse a rule from a specified input stream and return the resulting object.
 * @details  Read characters from the specified input stream and attempt to interpret
 * them as a rule.  A rule starts with a left square bracket '[', followed by a term,
 * then a comma ',' and a second term, and finally terminated by a right square bracket ']'.
 * Arbitrary whitespace may appear before a term or in between the atoms and punctuation
 * that make up the rule.  If, while parsing a rule, the first non-whitespace character
 * seen is not the required initial left square bracket, then the character read is
 * pushed back into the input stream and NULL is returned.  If, while parsing a rule,
 * the required commma ',' or right square bracket ']' is not seen, or parsing one of
 * the two subterms fails, then the unexpected character read is pushed back to the
 * input stream, an error message is issued (to stderr) and NULL is returned.
 * @param in  The stream from which characters are to be read.
 * @return  A pointer to the newly created rule, if parsing was successful,
 * otherwise NULL.
 */
REVERKI_RULE *reverki_parse_rule(FILE *in) {
    *reverki_atom_storage = *reverki_parse_atom(in);
    fseek(in, 0, SEEK_SET);
    int c, i=0, r=0, l=0, d=0;
    c= fgetc(in);
    while(c == 32 || c == 10){
        c= fgetc(in);
    }
    while(c == 91){
        //struct reverki_term lhs, rhs; *******************************
        reverki_parse_term(in);
        d=0;
        while((reverki_term_storage+d)->type != 0){
            d++;
        }
        l=d-1;

        reverki_parse_term(in);
        d=0;
        while((reverki_term_storage+d)->type != 0){
            d++;
        }
        r=d-1;

        reverki_make_rule((reverki_term_storage+l), (reverki_term_storage+r));
        c= fgetc(in);
        while(c == 32 || c == 10 || c == 93 || c == 41){
            c= fgetc(in);
        }
        i++;
    }
    fseek(in, -1, SEEK_CUR);

    if((global_options>>2)%2==1 && (global_options>>3)%2==1){
        d=0;
        while((reverki_rule_storage+d)->lhs != NULL){
            fprintf(stderr, "# ");
            reverki_unparse_rule((reverki_rule_storage+d), stderr);
            fprintf(stderr, "\n");
            d++;
        }
    }

    c=0;
    c = fgetc(stdin);
    while(c == 32 || c == 41 || c == 44 || c == 93 || c == 10 || c == 91){
        c = fgetc(in);
    }
    while(c==40){
        fseek(in, -1, SEEK_CUR);
        reverki_parse_term(in);
        d=0;
        while((reverki_term_storage+d)->type != 0){
            d++;
        }
        if((global_options>>2)%2==1 && (global_options>>3)%2==1){
            fprintf(stderr, "# ");
            reverki_unparse_term((reverki_term_storage+d-1), stderr);
            fprintf(stderr, "\n");
        }
        if((global_options>>2)%2==1){
            reverki_unparse_term(reverki_rewrite(NULL, (reverki_term_storage+d-1)), stdout);
            fprintf(stdout, "\n");
        }
        c=0;
        c = fgetc(in);
        while(c == 32 || c == 41 || c == 44 || c == 93 || c == 10 || c == 91){
            c = fgetc(in);
        }
        if(c==EOF){
            break;
        }
    }
    return NULL;
}

/*
 * @brief  Output a textual representation of a specified rule to a specified output stream.
 * @details  A textual representation of the specified rule is output to the specified
 * output stream.  The textual representation is of a form from which the original rule
 * can be reconstructed using reverki_parse_rule.  If the output is successful, then 0
 * is returned.  If any error occurs then the value EOF is returned.
 * @param rule  The rule to be printed.
 * @param out  Stream to which the rule is to be printed.
 * @return  0 if output was successful, EOF if not.
 */

int reverki_unparse_rule(REVERKI_RULE *rule, FILE *out) {
    fprintf(out, "[");
    reverki_unparse_term(rule->lhs, out);
    fprintf(out, ", ");
    reverki_unparse_term(rule->rhs, out);
    fprintf(out, "]");
    return 0;
}
