#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "reverki.h"
#include "global.h"

/**
 * @brief  This function rewrites a term, using a specified list of rules.
 * @details  The specified term is rewritten, using the specified list of
 * rules, until no more rewriting is possible (i.e. the result is a
 * "normal form" with respect to the rules).
 * Each rewriting step involves choosing one of the rules, matching the
 * left-hand side of the rule against a subterm of the term being rewritten,
 * and, if the match is successful, returning a new term obtained by
 * replacing the matched subterm by the term obtained by applying the
 * matching substitution to the right-hand side of the rule.
 *
 * The result of rewriting will in general depend on the order in which
 * the rules are tried, as well as the order in which subterms are selected
 * for matching against the rules.  These orders can affect whether the
 * rewriting procedure terminates, as well as what final term results from
 * the rewriting.  We will use a so-called "leftmost-innermost" strategy
 * for choosing the order in which rewriting occurs.  In this strategy,
 * subterms are recursively rewritten, in left-to-right order, before their
 * parent term is rewritten.  Once no further rewriting is possible for
 * subterms, rewriting of the parent term is attempted.  Since rewriting
 * of the parent term affects what subterms there are, each time the parent
 * term is changed rewriting is restarted, beginning again with recursive
 * rewriting of the subterms.  Rules are always tried in the order in which
 * they occur in the rule list: a rule occurring later in the list is
 * never used to rewrite a term unless none of the rules occurring earlier
 * can be applied.
 *
 * @param rule_list  The list of rules to be used for rewriting.
 * @param term  The term to be rewritten.
 * @return  The rewritten term.  This term should have the property that
 * no further rewriting will be possible on it or any of its subterms,
 * using rules in the specified list.
 */
REVERKI_TERM *new_term(REVERKI_TERM *old) {
    struct reverki_term *fst;
    struct reverki_term *snd;

    if(old->type == 3){
        fst = new_term(old->value.pair.fst);
        snd = new_term(old->value.pair.snd);
    }
    else{
        if(old->type == 1){
            return reverki_make_variable(old->value.atom);
        }
        if(old->type == 2){
            return reverki_make_constant(old->value.atom);
        }
    }

    return reverki_make_pair(fst, snd);
}

void print_left(REVERKI_TERM *term, int time){
    if(term->type == 3){
        int t=0;
        while(t < time){
            fprintf(stderr, ".");
            t++;
        }
        reverki_unparse_term(term->value.pair.fst, stderr);
        fprintf(stderr, "\n");
        print_left(term->value.pair.fst, time+1);
        t=0;
        while(t < time){
            fprintf(stderr, ".");
            t++;
        }
        reverki_unparse_term(term->value.pair.snd, stderr);
        fprintf(stderr, "\n");
        print_left(term->value.pair.snd, time+1);
    }
}

long limit = 1;
int match(REVERKI_TERM *pat, REVERKI_TERM *tgt, REVERKI_SUBST *substp, int time) {
    int d=0, rule =0;
    d=0;
    while((reverki_rule_storage+d)->lhs != NULL){
        d++;
    }
    rule = d-1;
    while(rule>-1){
        REVERKI_SUBST *sub = (REVERKI_SUBST *)malloc(sizeof(REVERKI_SUBST));
        int com = reverki_match(tgt, (reverki_rule_storage+rule)->lhs, sub);
        if(com){
            if((global_options>>5)%2==1){
                limit++;
            }
            if((global_options>>2)%2==1 && (global_options>>3)%2==1){
                int t=0;
                while(t < time){
                    fprintf(stderr, ".");
                    t++;
                }
                reverki_unparse_term(tgt,stderr);
                fprintf(stderr, "\n");
                fprintf(stderr, "==> rule: ");
                reverki_unparse_rule((reverki_rule_storage+rule),stderr);
                fprintf(stderr, ", subst: ");
            }
            new_term((reverki_rule_storage+rule)->rhs);
            int d=0;
            while((reverki_term_storage+d)->type != 0){
                d++;
            }
            tgt = (reverki_term_storage+d-1);
            while(com>0){
                com--;
                if((global_options>>2)%2==1 && (global_options>>3)%2==1){
                    reverki_unparse_rule(*(sub+com),stderr);
                    fprintf(stderr, " ");
                }
                reverki_apply(*(sub+com), tgt);
            }
            if((global_options>>2)%2==1 && (global_options>>3)%2==1){
                fprintf(stderr, ".\n");
            }
            free(sub);
            return d-1;
        }
        rule--;
    }
    return -1;
}

REVERKI_TERM *rewrite(int time, REVERKI_TERM *term) {
    if(term == NULL){
        return term;
    }

    int have=0;
    int again=1;
    void *temp;
    while(again){
        again=0;
        int ma = match(NULL, term, NULL, time);
        if(ma != -1){
            again = 1;
            have = 1;
            term = (reverki_term_storage+ma);
            if((global_options>>2)%2==1 && (global_options>>3)%2==1){
                int t=0;
                while(t < time){
                    fprintf(stderr, ".");
                    t++;
                }
                reverki_unparse_term(term, stderr);
                fprintf(stderr, "\n");
                print_left(term, time+1);
            }

            if(term->type == 3){
                temp = rewrite(time+1, term->value.pair.fst);
                if(temp != NULL){
                    again = 1;
                    term->value.pair.fst = temp;
                }

                temp = rewrite(time+1, term->value.pair.snd);
                if(temp != NULL){
                    again = 1;
                    term->value.pair.snd = temp;
                }
            }
        }
        else if(term->type == 3){
            temp = rewrite(time+1, term->value.pair.fst);
            if(temp != NULL){
                have = 1;
                again = 1;
                term->value.pair.fst = temp;
            }

            temp = rewrite(time+1, term->value.pair.snd);
            if(temp != NULL){
                have = 1;
                again = 1;
                term->value.pair.snd = temp;
            }
        }
    }
    if(have){
        return term;
    }
    return NULL;
}


REVERKI_TERM *reverki_rewrite(REVERKI_RULE *rule_list, REVERKI_TERM *term) {
    if((global_options>>2)%2==1 && (global_options>>3)%2==1){
        reverki_unparse_term(term, stderr);
        fprintf(stderr, "\n");
        print_left(term, 1);
    }
    void *temp = rewrite(0, term);
    if((global_options>>5)%2==1){
        long compare = global_options>>32;
        if(limit>compare){
            fprintf(stderr, "Rewrite limit exceeded.\n");
            abort();
        }
    }
    return temp;
}
