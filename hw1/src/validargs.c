#include <stdlib.h>

#include "reverki.h"
#include "global.h"
#include "debug.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */

int validargs(int argc, char **argv) {
    long options = 0;

    if(argc == 1 || **(argv+1) != '-' || *(*(argv+1)+2) != 0){
        return -1;
    }

    if(*(*(argv+1)+1) == 'h'){
        options = 1;
    }
    else{
        if(*(*(argv+1)+1) == 'v'){
            options += 2;


            if(argc == 2){
                global_options = options;
                return 0;
            }
            else if(**(argv+2) != '-' || *(*(argv+2)+2) != 0){
                return -1;
            }
            else if(*(*(argv+2)+1) == 's'){
                if(argc > 3){
                    return -1;
                }
                options += 16;
            }
            else{
                return -1;
            }
        }
        else if(*(*(argv+1)+1) == 'r'){
            options += 4;


            if(argc == 2){
                global_options = options;
                return 0;
            }
            else if(**(argv+2) != '-' || *(*(argv+2)+2) != 0){
                return -1;
            }
            else if(*(*(argv+2)+1) == 't'){
                if(argc > 3){
                    return -1;
                }
                options += 8;
            }
            else if(*(*(argv+2)+1) == 's'){
                if(argc > 3){
                    return -1;
                }
                options += 16;
            }
            else if(*(*(argv+2)+1) == 'l'){
                options += 32;
                if(argc == 4){
                    if(**(argv+3) == 48){
                        return -1;
                    }

                    int i=0;
                    while(*(*(argv+3)+i) != 0){
                        i++;
                    }

                    i--;
                    long max = 1;
                    max <<= 32;
                    long limit = 0;
                    long time = 1;
                    while(i>=0){
                        limit += ((*(*(argv+3)+i))-48) * time;
                        time *= 10;
                        i--;
                    }

                    if(limit < max){
                        limit <<= 32;
                        options += limit;
                    }
                    else{
                        return -1;
                    }
                }
                else{
                    return -1;
                }
            }
            else{
                return -1;
            }
        }
        else{
            return -1;
        }
    }
    global_options = options;
    return 0;
}
