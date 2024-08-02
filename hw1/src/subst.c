#include <stdlib.h>
#include <stdio.h>

#include "reverki.h"
#include "global.h"
#include "debug.h"

/**
 * @brief  Match a specified pattern term against a specified target term,
 * returning a substitution in case of a successful match.
 * @details  The pattern is compared with the target, by performing a simultaneous
 * recursive traversal of both.  Constants occurring in the pattern must match
 * exactly corresponding constants in the target.  Variables occurring in the
 * pattern may match any corresponding subterm of the target, except that if there
 * is more than one instance of a particular variable in the pattern, then all
 * instances of that variable must be matched to identical subterms of the target.
 * As the traversal proceeds, additional bindings of variables to subterms are
 * accumulated by prepending them to a substitution, which is accessed and
 * updated via a by-reference parameter.  If the match succeeds, a nonzero value
 * is returned and the final substitution can be obtained by the caller from this
 * by-reference parameter.  If the match should fail at some point, any partially
 * accumulated substitution is recycled, the by-reference parameter is reset to
 * NULL, and 0 is returned.
 * @param pat  The term to be used as the pattern.
 * @param tgt  The term to be used as the target.
 * @param substp  A by-reference parameter that is a pointer to a variable
 * containing a substitution.  The caller should declare such a variable,
 * initialize it to NULL, and pass its address to this function.
 * Upon nonzero return from this function, the variable will contain the
 * accumulated substitution that is the result of the matching procedure.
 * The caller is responsible for recycling this substitution once it is no
 * longer needed.  Upon zero return from this function this variable will
 * have value NULL and the caller should not attempt to use it.
 * @return  Nonzero if the match is successful, 0 otherwise.  If the match is
 * successful, then the by-reference parameter substp will contain a pointer
 * to the accumulated substitution.
 */
int compare_term(REVERKI_TERM *term1, REVERKI_TERM *term2, REVERKI_SUBST *sub) {
    //term2 is rule;
    if(term1->type == 0 || term2->type == 0){
        return 0;
    }

    if(term1->type == 3){
        if(term2->type == 2){
           return 0;
        }
        else if(term2->type == 1){
            int s=0;
            while((*(sub+s)) != NULL){
                s++;
            }
            (*(sub+s)) = (REVERKI_RULE *)malloc(sizeof(REVERKI_RULE));
            (*(sub+s))->lhs = term2;
            (*(sub+s))->rhs = term1;
            return 1;
        }
    }
    else{
        if(term1->value.atom == term2->value.atom){
            return -1;
        }
        else if(term2->type == 1){
            int s=0;
            while((*(sub+s)) != NULL){
                s++;
            }
            (*(sub+s)) = (REVERKI_RULE *)malloc(sizeof(REVERKI_RULE));
            (*(sub+s))->lhs = term2;
            (*(sub+s))->rhs = term1;
            return 1;
        }
        return 0;
    }

    int compare = 0;
    int com = compare_term(term1->value.pair.fst, term2->value.pair.fst, sub);
    if(com == 0){
        return 0;
    }
    else if(com > 0){
        compare+=com;
    }

    com = compare_term(term1->value.pair.snd, term2->value.pair.snd, sub);
    if(com == 0){
        return 0;
    }
    else if(com > 0){
        compare+=com;
    }

    if(compare == 0){
        return -1;
    }
    return compare;
}

int reverki_match(REVERKI_TERM *pat, REVERKI_TERM *tgt, REVERKI_SUBST *substp) {
        return compare_term(pat, tgt, substp);
}

/**
 * @brief  Apply a substitution to a term, producing a term, which in some cases
 * could be the same term as the argument.
 * @details  This function applies a substitution to a term and produces a result
 * term.  A substitution is applied to a term by recursively traversing the term
 * and, for each variable that is encountered, if that variable is one of the
 * key variables mapped by the substitution, replacing that variable by the
 * corresponding value term.  Because terms are immutable, if applying a
 * substitution results in a change to one of the subterms of a pair, then the
 * pair is not modified; rather, a new pair is constructed that contains the new
 * subterm and the other subterm that was not changed.
 * @param subst  The substitution to be applied.
 * @param term  The term to which to apply the substitution.
 * @return  The term constructed by applying the substitution to the term passed
 * as argument.
 */
REVERKI_TERM *reverki_apply(REVERKI_SUBST subst, REVERKI_TERM *term) {
    if(term->type != 3){
        if(term->value.atom == subst->lhs->value.atom){
            *term = *subst->rhs;
        }
        return term;
    }

    if(term->value.pair.fst->type != 3){
        if(term->value.pair.fst->value.atom == subst->lhs->value.atom){
            term->value.pair.fst = subst->rhs;
        }
    }
    else{
        reverki_apply(subst, term->value.pair.fst);
    }
    if(term->value.pair.snd->type != 3){
        if(term->value.pair.snd->value.atom == subst->lhs->value.atom){
            term->value.pair.snd = subst->rhs;
        }
    }
    else{
        reverki_apply(subst, term->value.pair.snd);
    }

    return term;
}
