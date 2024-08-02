#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "polya.h"

pid_t pid;
volatile sig_atomic_t cont_status;
volatile sig_atomic_t term_status;
volatile sig_atomic_t slove_status;
void worker_handler(int sign){
    if(sign==SIGCONT){
        cont_status=1;
        //kill(pid, SIGCONT);
    }
    else if(sign==SIGTERM){
        term_status=1;
        //kill(pid, SIGCONT);
    }
    else if(sign==SIGHUP){
        slove_status=1;
    }

        // case SIGCHLD:
        //     status=4;
}



/*
 * worker
 * (See polya.h for specification.)
 */
int worker(void) {
    // TO BE IMPLEMENTED
    //sigset_t mask_all, old_mask;
    //sigfillset(&mask_all);

    cont_status=0;
    term_status=0;
    slove_status=0;
    signal(SIGCONT, worker_handler);
    signal(SIGHUP, worker_handler);
    signal(SIGTERM, worker_handler);
    pid=getpid();
    struct result *res;
    //sigprocmask(SIG_BLOCK, &mask_all, &old_mask);
    // int fd[2],pid;
    // FILE *in, *out;
    while(1){
        slove_status=0;
        kill(pid, SIGSTOP);
        if(term_status==1){
            //close(fd[0]);
            //fprintf(stderr, "\n12344567890\n");
            //kill(getpid(), SIGKILL);
            exit(0);
        }
        //sigprocmask(SIG_SETMASK, &old_mask, NULL);
        //fprintf(stderr, "\n12347890\n");
        // char test[10];
        // //fgets(test, 5, stdin);
        // read(STDIN_FILENO, &test, 5);
        //fprintf(stderr, "\n%s\n", test);
        if(cont_status==1){
            cont_status=0;
            //slove_status=0;
            //void *size= (void *)malloc(9);
            char sizeA[8];
            //fgets(size, 9, stdin);
            read(STDIN_FILENO, sizeA, 8);
            size_t size=0;
            int i=0;
            int j=1;
            while(i<8 && sizeA[i]!=0){
                size+=sizeA[i]*j;
                i++;
                j*=10;
            }
            void *pro= (void *)malloc((size_t)size);
            (((struct problem *)pro)->size) = (size_t)size;
            //fprintf(stderr, "\n%ld\n", (size_t)size);
            //fgets(pro+8, ((size_t)size)-7, stdin);
            read(STDIN_FILENO, pro+8, ((size_t)size)-8);

            //free(size);
            // if(pipe(fd) < 0) {
            //     perror("Can't create pipe1");
            //     exit(1);
            // }
            // if((pid=fork())==0){
            //     close(fd[0]);
            //     if((out = fdopen(fd[1], "w")) == NULL) {
            //         perror("Child can't create output1 stream\n");
            //         exit(1);
            //     }
            //     //out1, child write to parent
            //     struct result *res=(solvers[(((struct problem *)pro)->type)].solve)(pro, 0);
            //     fputs((void *)res,out);
            //     close(fd[1]);
            //     exit(0);
            // }
            // else{
            //     close(fd[1]);
            //     if((in = fdopen(fd[0], "r")) == NULL) {
            //         perror("Parent can't create input1 stream\n");
            //         exit(1);
            //     }
            //     //in1, parent read from child
            //     free(pro);
            // }
            res=(solvers[(((struct problem *)pro)->type)].solve)(pro, &slove_status);
            if(slove_status!=0){
                free(res);
                res=(struct result *)malloc(sizeof(struct result));
                res->size=sizeof(struct result);
                res->failed=1;
                write(STDOUT_FILENO, res, ((struct result *)res)->size);
            }
            else{
                write(STDOUT_FILENO, res, ((struct result *)res)->size);
            };
            free(pro);
            free(res);
            //sigprocmask(SIG_BLOCK, &mask_all, &old_mask);
        }



        // if(status==4){

        //     kill(getpid(), SIGSTOP);
        // }
        //sigsuspend (&old_mask);
    }
}
