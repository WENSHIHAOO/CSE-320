#include <stdlib.h>
#include <stdio.h>

#include "reverki.h"
#include "global.h"
#include "debug.h"

/*
 * @brief  Create a variable term from a specified atom.
 * @details  A term of type REVERKI_VARIABLE is created that contains the
 * specified atom.  The atom must also have type REVERKI_VARIABLE, otherwise
 * an error message is printed and the program aborts.
 * @param atom  The atom from which the term is to be constructed.
 * @return  A pointer to the newly created term.
 */
REVERKI_TERM *reverki_make_variable(REVERKI_ATOM *atom) {
    if(atom->type != REVERKI_VARIABLE_TYPE){
        fprintf(stderr, "The atom NOT have type REVERKI_VARIABLE");
        abort();
    }

    int d=0;
    while((reverki_term_storage+d)->type != 0){
        d++;
    }
    struct reverki_term *term = (reverki_term_storage+d);
    term->type = REVERKI_VARIABLE_TYPE;
    term->value.atom = atom;

    return term;
}

/*
 * @brief  Create a constant term from a specified atom.
 * @details  A term of type REVERKI_CONSTANT is created that contains the
 * specified atom.  The atom must also have type REVERKI_CONSTANT, otherwise
 * an error message is printed and the program aborts.
 * @param atom  The atom from which the constant is to be constructed.
 * @return  A pointer to the newly created term.
 */
REVERKI_TERM *reverki_make_constant(REVERKI_ATOM *atom) {
    if(atom->type != REVERKI_CONSTANT_TYPE){
        fprintf(stderr, "The atom NOT have type REVERKI_CONSTANT");
        abort();
    }

    int d=0;
    while((reverki_term_storage+d)->type != 0){
        d++;
    }
    struct reverki_term *term = (reverki_term_storage+d);
    term->type = REVERKI_CONSTANT_TYPE;
    term->value.atom = atom;

    return term;
}

/*
 * @brief  Create a pair term from specified subterms.
 * @details  A term of type REVERKI_PAIR is created that contains specified
 * terms as its first and second subterms.
 * @param fst  The first (or "left-hand") subterm of the pair to be constructed.
 * @param snd  The second (or "right-hand") subterm of the pair to be constructed.
 * @return  A pointer to the newly created term.
 */
REVERKI_TERM *reverki_make_pair(REVERKI_TERM *fst, REVERKI_TERM *snd) {
    int d=0;
    while((reverki_term_storage+d)->type != 0){
        d++;
    }
    struct reverki_term *term = (reverki_term_storage+d);
    term->type = REVERKI_PAIR_TYPE;
    term->value.pair.fst = fst;
    term->value.pair.snd = snd;
    return term;
}

/*
 * @brief  Compare two specified terms for equality.
 * @details  The two specified terms are compared for equality.  Equality of terms
 * means that they have the same type and that corresponding atoms or subterms they
 * contain are recursively equal.
 * @param term1  The first of the two terms to be compared.
 * @param term2  The second of the two terms to be compared.
 * @return  Zero if the specified terms are equal, otherwise nonzero.
 */
int reverki_compare_term(REVERKI_TERM *term1, REVERKI_TERM *term2) {
    //term2 is rule;
    if(term1->type == 3){
        if(term2->type == 2){
           return 0;
        }
        return 1;
    }
    else{
        if(term1->type == term2->type){
            if(term1->value.atom == term2->value.atom){
                return 1;
            }
        }
        return 0;
    }


    int compare = 1;
    compare = reverki_compare_term(term1->value.pair.fst, term2->value.pair.fst);
    if(compare == 0){
        return 0;
    }
    compare = reverki_compare_term(term1->value.pair.snd, term2->value.pair.snd);
    return compare;
}

/*
 * @brief  Parse a term from a specified input stream and return the resulting object.
 * @details  Read characters from the specified input stream and attempt to interpret
 * them as a term.  A term may either have the form of a single atom, or of a
 * sequence of terms enclosed by left parenthesis '(' and right parenthesis ')'.
 * Arbitrary whitespace may appear before a term or in between the atoms and punctuation
 * that make up the term.  Any such whitespace is not part of the term, and is ignored.
 * When a whitespace or punctuation character is encountered that signals the end of
 * the term, this character is pushed back into the input stream.  If the term is
 * terminated due to EOF, no character is pushed back.  A term of the form
 * ( a b c d ... ) is regarded as an abbreviation for ( ... ( ( a b ) c ) d ) ...);
 * that is, parentheses in a term may be omitted under the convention that the subterms
 * associate to the left.  If, while reading a term, a syntactically incorrect atom
 * or improperly matched parentheses are encountered, an error message is issued
 * (to stderr) and NULL is returned.
 * @param in  The stream from which characters are to be read.
 * @return  A pointer to the newly created term, if parsing was successful,
 * otherwise NULL.
 */
int count(FILE *in){
    int c =0, num =0, seek=0, parentheses = 0;

    while(c != EOF && c != 44 && c != 93 && parentheses > -1){
        c=fgetc(in);

        if(c == 40){
            parentheses++;
        }
        else if(c == 41){
            parentheses--;
        }
        else if(parentheses==0 && c == 32){
            num++;
        }

        seek++;
    }

    fseek(in, -1*seek, SEEK_CUR);

    if(num>1){
        return num-1;
    }
    else{
        return 0;
    }
}

REVERKI_TERM *reverki_parse_term0(FILE *in, int count1) {
    struct reverki_term *fst;
    struct reverki_term *snd;
    int c=0;

    if(count1 > 0){
        fst = reverki_parse_term0(in, --count1);
    }
    else{
        c = fgetc(in);
        while(c == 32 || c == 41){
            c = fgetc(in);
            if(c == EOF){
                return NULL;
            }
        }

        if(c==40){
            count1 = count(in);
            fst = reverki_parse_term0(in, count1);
        }
        else if(c>96&&c<123){
            int i=0, j=0;
            while(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
                *(reverki_pname_buffer+i) = (char)c;
                i++;
                c = fgetc(in);
            }
            *(reverki_pname_buffer+i) = '\0';
            struct reverki_atom temp = *reverki_atom_storage;
            int have =0;
            while(temp.next != NULL){
                i=0;
                while(*(temp.pname+i) != 0 && *(reverki_pname_buffer+i) !=0){
                    if(*(temp.pname+i) != *(reverki_pname_buffer+i)){
                        break;
                    }

                    i++;
                    if(*(temp.pname+i) == 0 && *(reverki_pname_buffer+i) ==0){
                        have = 1;
                    }
                }

                if(have){
                    break;
                }
                temp = *temp.next;
                j++;
            }
            fst = reverki_make_variable(reverki_atom_storage+j);
        }
        else if(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
            int i=0, j=0;
            while(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
                *(reverki_pname_buffer+i) = (char)c;
                i++;
                c = fgetc(in);
            }
            *(reverki_pname_buffer+i) = '\0';
            struct reverki_atom temp = *reverki_atom_storage;
            int have =0;
            while(temp.next != NULL){
                i=0;
                while(*(temp.pname+i) != 0 && *(reverki_pname_buffer+i) !=0){
                    if(*(temp.pname+i) != *(reverki_pname_buffer+i)){
                        break;
                    }

                    i++;
                    if(*(temp.pname+i) == 0 && *(reverki_pname_buffer+i) ==0){
                        have = 1;
                    }
                }

                if(have){
                    break;
                }
                temp = *temp.next;
                j++;
            }
            fst = reverki_make_constant(reverki_atom_storage+j);
        }
    }

    c = fgetc(in);
    while(c == 32 || c == 41){
        c = fgetc(in);
        if(c == EOF){
            return NULL;
        }
    }

    if(c==40){
        count1 = count(in);
        snd = reverki_parse_term0(in, count1);
    }
    else if(c>96&&c<123){
        int j=0;
        int i=0;
        while(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
            *(reverki_pname_buffer+i) = (char)c;
            i++;
            c = fgetc(in);
        }
        *(reverki_pname_buffer+i) = '\0';
        struct reverki_atom temp = *reverki_atom_storage;
        int have =0;
        while(temp.next != NULL){
            i=0;
            while(*(temp.pname+i) != 0 && *(reverki_pname_buffer+i) !=0){
                if(*(temp.pname+i) != *(reverki_pname_buffer+i)){
                    break;
                }

                i++;
                if(*(temp.pname+i) == 0 && *(reverki_pname_buffer+i) ==0){
                    have = 1;
                }
            }

            if(have){
                break;
            }
            temp = *temp.next;
            j++;
        }
        snd = reverki_make_variable(reverki_atom_storage+j);
    }
    else if(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
        int i=0, j=0;
        while(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
            *(reverki_pname_buffer+i) = (char)c;
            i++;
            c = fgetc(in);
        }
        *(reverki_pname_buffer+i) = '\0';
        struct reverki_atom temp = *reverki_atom_storage;
        int have =0;
        while(temp.next != NULL){
            i=0;
            while(*(temp.pname+i) != 0 && *(reverki_pname_buffer+i) !=0){
                if(*(temp.pname+i) != *(reverki_pname_buffer+i)){
                    break;
                }

                i++;
                if(*(temp.pname+i) == 0 && *(reverki_pname_buffer+i) ==0){
                    have = 1;
                }
            }

            if(have){
                break;
            }
            temp = *temp.next;
            j++;
        }
        snd = reverki_make_constant(reverki_atom_storage+j);
    }

    return reverki_make_pair(fst, snd);
}

REVERKI_TERM *reverki_parse_term(FILE *in) {
    int c=0;
    c = fgetc(in);
    while(c == 32 || c == 41 || c == 44 || c == 93 || c == 10 || c == 91){
        c = fgetc(in);
    }

    if(c == 40){
        int count1 = count(in);
        return  reverki_parse_term0(in, count1);
    }
    else if(c>96&&c<123){
        int i=0, j=0;
        while(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
            *(reverki_pname_buffer+i) = (char)c;
            i++;
            c = fgetc(in);
        }
        fseek(in, -1 , SEEK_CUR);
        *(reverki_pname_buffer+i) = '\0';
        struct reverki_atom temp = *reverki_atom_storage;
        int have =0;
        while(temp.next != NULL){
            i=0;
            while(*(temp.pname+i) != 0 && *(reverki_pname_buffer+i) !=0){
                if(*(temp.pname+i) != *(reverki_pname_buffer+i)){
                    break;
                }

                i++;
                if(*(temp.pname+i) == 0 && *(reverki_pname_buffer+i) ==0){
                    have = 1;
                }
            }

            if(have){
                break;
            }
            temp = *temp.next;
            j++;
        }
        return reverki_make_variable(reverki_atom_storage+j);
    }
    else if(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
        int i=0, j=0;
        while(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
            *(reverki_pname_buffer+i) = (char)c;
            i++;
            c = fgetc(in);
        }
        fseek(in, -1 , SEEK_CUR);
        *(reverki_pname_buffer+i) = '\0';
        struct reverki_atom temp = *reverki_atom_storage;
        int have =0;
        while(temp.next != NULL){
            i=0;
            while(*(temp.pname+i) != 0 && *(reverki_pname_buffer+i) !=0){
                if(*(temp.pname+i) != *(reverki_pname_buffer+i)){
                    break;
                }

                i++;
                if(*(temp.pname+i) == 0 && *(reverki_pname_buffer+i) ==0){
                    have = 1;
                }
            }

            if(have){
                break;
            }
            temp = *temp.next;
            j++;
        }
        return reverki_make_constant(reverki_atom_storage+j);
    }
    else{
        return NULL;
    }
}
/*
 * @brief  Output a textual representation of a specified term to a specified output stream.
 * @details  A textual representation of the specified term is output to the specified
 * output stream.  The textual representation is of a form from which the original term
 * can be reconstructed using reverki_parse_term.  If the output is successful, then 0
 * is returned.  If any error occurs then the value EOF is returned.
 * @param term  The term that is to be printed.
 * @param out  Stream to which the term is to be printed.
 * @return  0 if output was successful, EOF if not.
 */

int reverki_unparse_term(REVERKI_TERM *term, FILE *out) {
    if(term->type == REVERKI_PAIR_TYPE){
        fprintf(out, "(");
        if(term->value.pair.fst->type == REVERKI_PAIR_TYPE){
            if(term->value.pair.fst == NULL){
                return 0;
            }
            if(reverki_unparse_term(term->value.pair.fst->value.pair.fst, out) == EOF){
                return EOF;
            }

            fprintf(out, " ");
            if(term->value.pair.fst->value.pair.snd == NULL){
                return 0;
            }
            if(reverki_unparse_term(term->value.pair.fst->value.pair.snd, out) == EOF){
                return EOF;
            }
        }
        else if(term->value.pair.fst->value.pair.fst->type == 0){
            return 0;
        }
        else{
            if(reverki_unparse_atom(term->value.pair.fst->value.atom, out) == EOF){
                return EOF;
            }
        }

        fprintf(out, " ");
        if(term->value.pair.snd == NULL){
            return 0;
        }
        if(reverki_unparse_term(term->value.pair.snd, out) == EOF){
            return EOF;
        }
        fprintf(out, ")");
    }
    else if(term->type == 0){
        return 0;
    }
    else{
        if(reverki_unparse_atom(term->value.atom, out) == EOF){
            return EOF;
        }
    }

    return 0;
}
