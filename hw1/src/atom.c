#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "reverki.h"
#include "global.h"

/*
 * @brief  Parse an atom  from a specified input stream and return the resulting object.
 * @details  Read characters from the specified input stream and attempt to interpret
 * them as an atom.  An atom may start with any non-whitespace character that is not
 * a left '(' or right '(' parenthesis, a left '[' or right ']' square bracket,
 * or a comma ','.  If the first character read is one of '(', ')', '[', ']', then it
 * is not an error; instead, the character read is pushed back into the input stream
 * and NULL is returned.  Besides the first character, an atom may consist
 * of any number of additional characters (other than whitespace and the punctuation
 * mentioned previously), up to a maximum length of REVERKI_PNAME_BUFFER_SIZE-1.
 * If this maximum length is exceeded, then an error message is printed (on stderr)
 * and NULL is returned.  When a whitespace or punctuation character is encountered
 * that signals the end of the atom, this character is pushed back into the input
 * stream.  If the atom is terminated due to EOF, no character is pushed back.
 * An atom that starts with a lower-case letter has type REVERKI_VARIABLE_TYPE,
 * otherwise it has type REVERKI_CONSTANT_TYPE.  An atom is returned having the
 * sequence of characters read as its pname, and having the type determined by this
 * pname.  There can be at most one atom having a given pname, so if the pname read
 * is already the pname of an existing atom, then a pointer to the existing atom is
 * returned; otherwise a pointer to a new atom is returned.
 * @param in  The stream from which characters are to be read.
 * @return  A pointer to the atom, if the parse was successful, otherwise NULL if
 * an error occurred.
 */
REVERKI_ATOM *reverki_parse_atom(FILE *in) {
static struct reverki_atom head;
    int c;
    while(head.type == 0){
        c = fgetc(in);
        if(c == EOF){
            return NULL;
        }
        else if(c>96&&c<123){
            head.type = REVERKI_VARIABLE_TYPE;
            int i=0;
            while(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
                if(i>63){
                    fprintf(stderr, "Maximum length of pname is exceeded");
                    return NULL;
                }
                *(head.pname+i) = (char)c;
                i++;
                c = fgetc(in);
            }
            *(head.pname+i) = '\0';
        }
        else if(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
            head.type = REVERKI_CONSTANT_TYPE;
            int i=0;
            while(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
                if(i>63){
                    fprintf(stderr, "Maximum length of pname is exceeded");
                    return NULL;
                }
                *(head.pname+i) = (char)c;
                i++;
                c = fgetc(in);
            }
            *(head.pname+i) = '\0';
        }
    }

    int j=1;
    struct reverki_atom *Next = reverki_atom_storage+j;
    j++;
    head.next = Next;

    struct reverki_atom temp;
    while((c=fgetc(in)) != EOF){
        if(c>96&&c<123){
            Next->type = REVERKI_VARIABLE_TYPE;
            int i=0;
            while(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
                if(i>63){
                    fprintf(stderr, "Maximum length of pname is exceeded");
                    return NULL;
                }
                *(Next->pname+i) = (char)c;
                i++;
                c = fgetc(in);
            }
            *(Next->pname+i) = '\0';
        }
        else if(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
            Next->type = REVERKI_CONSTANT_TYPE;
            int i=0;
            while(c != 32&&c != 40&&c != 41&&c != 44&&c != 93&&c != 10&&c != 91){
                if(i>63){
                    fprintf(stderr, "Maximum length of pname is exceeded");
                    return NULL;
                }
                *(Next->pname+i) = (char)c;
                i++;
                c = fgetc(in);
            }
            *(Next->pname+i) = '\0';
        }

        if(*(Next->pname) != 0){
            temp = head;
            int have = 0;
            while(temp.next != NULL){
                int i=0;
                while(*(temp.pname+i) != 0 && *(Next->pname+i) !=0){
                    if(*(temp.pname+i) != *(Next->pname+i)){
                        break;
                    }

                    i++;
                    if(*(temp.pname+i) == 0 && *(Next->pname+i) ==0){
                        have = 1;
                    }
                }

                if(have){
                    break;
                }
                temp = *temp.next;
            }

            if(!have){
                struct reverki_atom *Next0 =reverki_atom_storage+j;
                j++;
                Next->next = Next0;
                Next = Next0;
            }
        }
    }

    return &head;
}

/*
 * @brief  Output the pname of a specified atom to the specified output stream.
 * @details  The pname of the specified atom is output to the specified output stream.
 * If the output is successful, then 0 is returned.  If any error occurs then the
 * value EOF is returned.
 * @param atom  The atom whose pname is to be printed.
 * @param out  Stream to which the pname is to be printed.
 * @return  0 if output was successful, EOF if not.
 */
int reverki_unparse_atom(REVERKI_ATOM *atom, FILE *out) {
    if(atom->type == 0){
        return EOF;
    }

    if(atom != NULL){
      fprintf(out,"%s",atom->pname);
    }
    return 0;
}
