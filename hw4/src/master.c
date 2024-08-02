#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "debug.h"
#include "polya.h"

struct worker_status {
    volatile sig_atomic_t pid;
    volatile sig_atomic_t status;
};
// // struct problem_pid{
// //     short id;
// //     int pid;
// //     short type;
// // };

struct worker_status workers_status[32];
int workers_num=0;
void child_handler(int sig) {
    int status=0;
    int pid=0;
    int i=0;

    if((pid=waitpid(-1, &status, WNOHANG|WUNTRACED)) > 0){
        for(i=0;i<workers_num;i++){
            if(workers_status[i].pid==0) {
                workers_status[i].pid=pid;
                workers_status[i].status=status;
                break;
            }
        }
    }
}

/*
 * master
 * (See polya.h for specification.)
 */
int master(int workers) {
    // TO BE IMPLEMENTED
    workers_num=workers;
    sf_suppress_chatter=0;
    sf_start();
    //workers_num=workers;

    //signal(SIGCHLD, child_handler);
    int fd1[2],fd2[2];
    int pid[workers];
    //FILE *in, *out;

    if(pipe(fd1) < 0) {
        perror("Can't create pipe1");
        exit(1);
    }
    if(pipe(fd2) < 0) {
        perror("Can't create pipe2");
        exit(1);
    }

    int i;
    // for(i=0; i<workers; i++){
    //     workers_status[i].stopOrterm=0;
    //     workers_status[i].prv_status=0;
    // }
    sigset_t mask_child, mask;
    sigemptyset(&mask_child);
    sigaddset(&mask_child, SIGCHLD);
    for(i=0; i<workers; i++){
        if((pid[i]=fork())==0) {
            //child process
            close(fd1[1]);
            close(fd2[0]);
            if(fdopen(fd1[0], "r") == NULL) {
                perror("Child can't create input1 stream\n");
                close(fd1[0]);
                close(fd2[1]);
                exit(1);
            }
            if(fdopen(fd2[1], "w") == NULL) {
                perror("Child can't create output2 stream\n");
                close(fd1[0]);
                close(fd2[1]);
                exit(1);
            }
            dup2(fd1[0], STDIN_FILENO);
            //fd1[0], child read from parent
            dup2(fd2[1], STDOUT_FILENO);
            //fd2[1], child write to parent
            // char test[10];
            // //fgets(test, 5, stdin);
            // read(STDIN_FILENO, &test, 5);
            // fprintf(stderr, "\n%s\n", test);
            char *args[] = {"bin/polya_worker", NULL};
            //execv(binaryPath, args);
            sf_suppress_chatter=0;
            if (execve(args[0], args, NULL) < 0) {
                printf("%s: BinaryPath not found.\n", args[0]);
                close(fd1[0]);
                close(fd2[1]);
                exit(1);
            }

            //close(fd[1]);
            close(fd1[0]);
            close(fd2[1]);
            exit(0);
        }
        else if(pid[i]<0){
            perror("fork() error");
            close(fd1[1]);
            close(fd2[0]);
            close(fd1[0]);
            close(fd2[1]);
            exit(1);
        }
    }
    //parent process

    int pid0=0;
    i=0;
    while(i<workers){
        if((pid0=waitpid(-1, NULL, WUNTRACED)) > 0){
            i++;
        }
    }

    close(fd1[0]);
    close(fd2[1]);
    if(fdopen(fd1[1], "w") == NULL) {
        perror("Parent can't create output1 stream\n");
        close(fd1[1]);
        close(fd2[0]);
        exit(1);
    }
    if(fdopen(fd2[0], "r") == NULL) {
        perror("Child can't create input2 stream\n");
        close(fd1[1]);
        close(fd2[0]);
        exit(1);
    }
    dup2(fd1[1], STDOUT_FILENO);
    //fd1[1], parent write to child
    dup2(fd2[0], STDIN_FILENO);
    //fd2[0], parent read from child
    // int ppi_size=10;
    // struct problem_pid *pp=(struct problem_pid *)malloc(sizeof(struct problem_pid)*ppi_size);
    // int ppi=0;
    struct problem *pro;
    int brea=0;
    int init=0;
    signal(SIGCHLD, child_handler);
    while(1){
        for(i=0; i<workers; i++){
            workers_status[i].pid=0;
        }
        for(i=0; i<workers; i++){
            //started or stopped
            if(init==0){
                sf_suppress_chatter=0;
                sf_change_state(pid[i], 0, WORKER_STARTED);
                sf_change_state(pid[i], WORKER_STARTED, WORKER_IDLE);
            }

            //idle
            pro=get_problem_variant(workers, i);
            if(pro==NULL){
                brea=1;
                break;
            }
            //pp[ppi].id=pro->id;
            //pp[ppi].pid=workers_status[i].pid;
            //ppi++;

            // if(ppi==ppi_size){
            //     ppi_size+=10;
            //     pp=(struct problem_pid *)realloc(pp, sizeof(struct problem_pid)*ppi_size);
            // }

            //continue and sent
            sf_suppress_chatter=0;
            sf_change_state(pid[i], WORKER_IDLE, WORKER_CONTINUED);
            sf_suppress_chatter=0;
            sf_send_problem(pid[i], pro);
            write(STDOUT_FILENO, pro, pro->size);
            kill(pid[i], SIGCONT);
            //running
            sf_suppress_chatter=0;
            sf_change_state(pid[i], WORKER_CONTINUED, WORKER_RUNNING);
        }
        if(brea){
            break;
        }

        int corrct=0;
        int j=0;
        i=0;
        while(corrct==0){
            while(corrct==0 && i<workers){
                if(workers_status[i].pid>0){
                    pid0=workers_status[i].pid;
                    sf_suppress_chatter=0;
                    sf_change_state(pid0, WORKER_RUNNING, WORKER_STOPPED);
                    //check
                    char sizeA[8];
                    read(STDIN_FILENO, sizeA, 8);
                    size_t size=0;
                    int x=0;
                    j=1;
                    while(x<8 && sizeA[x]!=0){
                        size+=sizeA[x]*j;
                        x++;
                        j*=10;
                    }
                    void *res= (void *)malloc((size_t)size);
                    (((struct result *)res)->size) = (size_t)size;
                    read(STDIN_FILENO, res+8, ((size_t)size)-8);
                    sf_suppress_chatter=0;
                    sf_recv_result(pid0, res);
                    if((solvers[pro->type].check)(res, pro)==0){
                        corrct=1;
                        post_result(res, pro);
                        sf_suppress_chatter=0;
                        sf_change_state(pid0, WORKER_STOPPED, WORKER_IDLE);
                    }
                    else{
                        sf_suppress_chatter=0;
                        sf_change_state(pid0, WORKER_STOPPED, WORKER_IDLE);
                    }
                    free(res);
                    i++;
                }
                else{
                    break;
                }
            }

            if(corrct==0){
                if(workers==i){
                    exit(1);
                }
                sigprocmask (SIG_BLOCK, &mask_child, &mask);
                sigsuspend(&mask);
                sigprocmask (SIG_SETMASK, &mask, NULL);
            }
        }



        for(j=0;j<workers; j++){
            if(pid[j]!=pid0){
                kill(pid[j], SIGHUP);
            }
        }

        while(i<workers){
            if(workers_status[i].pid>0){
                sf_suppress_chatter=0;
                sf_cancel(workers_status[i].pid);
                sf_suppress_chatter=0;
                sf_change_state(workers_status[i].pid, WORKER_RUNNING, WORKER_STOPPED);
                //check
                char sizeA[8];
                read(STDIN_FILENO, sizeA, 8);
                size_t size=0;
                int x=0;
                j=1;
                while(x<8 && sizeA[x]!=0){
                    size+=sizeA[x]*j;
                    x++;
                    j*=10;
                }
                void *res= (void *)malloc((size_t)size);
                (((struct result *)res)->size) = (size_t)size;
                read(STDIN_FILENO, res+8, ((size_t)size)-8);
                sf_suppress_chatter=0;
                sf_recv_result(workers_status[i].pid, res);
                sf_suppress_chatter=0;
                sf_change_state(workers_status[i].pid, WORKER_STOPPED, WORKER_IDLE);
                free(res);
                i++;
            }

            if(i<workers && workers_status[i].pid==0){
                sigprocmask (SIG_BLOCK, &mask_child, &mask);
                sigsuspend(&mask);
                sigprocmask (SIG_SETMASK, &mask, NULL);
            }
        }
        //int pid1=0;
        // j=0;
        // for(j=0;j<workers; j++){
        //     if(pid[j]!=pid0){
        //         sf_suppress_chatter=0;
        //         sf_cancel(pid[j]);
        //         kill(pid[j], SIGHUP);
        //         kill(pid[j], SIGCONT);
        //         kill(pid[j], SIGSTOP);
        //         if((pid1=waitpid(-1, NULL, WUNTRACED)) > 0){
        //             sf_suppress_chatter=0;
        //             sf_change_state(pid1, WORKER_RUNNING, WORKER_STOPPED);
        //             //check
        //             char sizeA[8];
        //             read(STDIN_FILENO, sizeA, 8);
        //             size_t size=0;
        //             int x=0;
        //             j=1;
        //             while(x<8 && sizeA[x]!=0){
        //                 size+=sizeA[x]*j;
        //                 x++;
        //                 j*=10;
        //             }
        //             void *res= (void *)malloc((size_t)size);
        //             (((struct result *)res)->size) = (size_t)size;
        //             read(STDIN_FILENO, res+8, ((size_t)size)-8);
        //             sf_suppress_chatter=0;
        //             sf_recv_result(pid1, res);
        //             sf_suppress_chatter=0;
        //             sf_change_state(pid1, WORKER_STOPPED, WORKER_IDLE);
        //             free(res);
        //         }
        //     }
        // }
        init=1;
    }

    int status;
    for(i=0;i<workers;i++){
        kill(pid[i], SIGTERM);
        kill(pid[i], SIGCONT);
        waitpid(pid[i], &status, WUNTRACED);
        if(WIFEXITED(status)){
            if(WEXITSTATUS(status)!=0){
                sf_suppress_chatter=0;
                sf_end();
                close(fd1[1]);
                close(fd2[0]);
                exit(1);
            }
            workers_status[0].pid=0;
            sf_suppress_chatter=0;
            sf_change_state(pid[i], WORKER_IDLE, WORKER_CONTINUED);
            sf_suppress_chatter=0;
            sf_change_state(pid[i], WORKER_CONTINUED, WORKER_RUNNING);
            sf_suppress_chatter=0;
            sf_change_state(pid[i], WORKER_RUNNING, WORKER_EXITED);
        }
        else{
            sf_suppress_chatter=0;
            sf_end();
            close(fd1[1]);
            close(fd2[0]);
            exit(1);
        }
    }
    sf_suppress_chatter=0;
    sf_end();
    close(fd1[1]);
    close(fd2[0]);
    exit(0);
}
