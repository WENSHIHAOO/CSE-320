#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    //double* ptr = sf_malloc(sizeof(double));


    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    sf_errno = 0;
    void *x = sf_memalign(40, 512);


    fprintf(stderr,"\n%p\n", x);
    fprintf(stderr,"\n%ld\n", ((((sf_block*)(x-8))->header>>5)*32-56)/32);
    fprintf(stderr,"\n%d\n", sf_errno);
    fprintf(stderr,"\n*****************\n");
    sf_show_blocks();
    fprintf(stderr,"\n*****************\n");
    sf_show_free_lists();
    fprintf(stderr,"\n*****************\n");
    sf_show_heap();

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
