#include <stdio.h>
#include <stdlib.h>

#include "reverki.h"
#include "global.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if(global_options == HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);

    // if((global_options>>1)%2==1){printf("1\n");}// VALIDATE_OPTION(-v)
    // if((global_options>>2)%2==1){printf("2\n");}// REWRITE_OPTION(-r)
    // if((global_options>>2)%2==1 && (global_options>>3)%2==1){printf("3\n");}// TRACE_OPTION(-r -t)
    // if((global_options>>1)%2==1 && (global_options>>4)%2==1){printf("4\n");}// VALIDATE_OPTION, STATISTICS_OPTION(-v -s)
    // if((global_options>>2)%2==1 && (global_options>>4)%2==1){printf("5\n");}// REWRITE_OPTION, STATISTICS_OPTION(-r -s)
    // if((global_options>>2)%2==1 && (global_options>>5)%2==1){printf("6\n");}//REWRITE_OPTION, LIMIT_OPTION(-r -l)
    // //limit = global_options>>32;
    if((global_options>>1)%2==1 || (global_options>>2)%2==1){
        reverki_parse_rule(stdin);
    }

    if((global_options>>4)%2==1){
        int d=0;
        while((reverki_atom_storage+d)->next != NULL){
            d++;
        }
        fprintf(stderr, "Atoms used: %d", d);
        fprintf(stderr, ", free: %d\n", 100-d);
        //
        d=0;
        while((reverki_term_storage+d)->type != 0){
            d++;
        }
        fprintf(stderr, "Terms used: %d", d);
        fprintf(stderr, ", free: %d\n", 10000-d);
        //
        d=0;
        while((reverki_rule_storage+d)->lhs != NULL){
            d++;
        }
        fprintf(stderr, "Rules used: %d", d);
        fprintf(stderr, ", free: %d\n", 1000-d);

    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */