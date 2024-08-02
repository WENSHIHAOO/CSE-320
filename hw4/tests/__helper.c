#include <criterion/criterion.h>
#include <criterion/logging.h>

#include "polya.h"
#include "event.h"
#include "tracker.h"
#include "__helper.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "debug.h"

char *worker_state_names[] = {
    [0]			"init",
    [WORKER_STARTED]   	"started",
    [WORKER_IDLE]	"idle",
    [WORKER_CONTINUED]  "continued",
    [WORKER_RUNNING]    "running",
    [WORKER_STOPPED]    "stopped",
    [WORKER_EXITED]     "exited",
    [WORKER_ABORTED]	"aborted"
};

/*
 * Read from an input stream a block of data of a specified number of bytes.
 */
int read_data_t(FILE *in, char *resultp, int nbytes) {
    debug("[%d:      ] Read data (fd = %d, nbytes = %d)", getpid(), fileno(in), nbytes);
    for(int i = 0; i < nbytes; i++) {
        int c;
        if((c = fgetc(in)) == EOF)
            return EOF;
        *resultp++ = c & 0xff;
        //debug("[%d] read 0x%x", getpid(), c);
    }
    return nbytes;
}

/*
 * Write to an output stream a block of data of a specified number of bytes.
 */
int write_data_t(FILE *out, char *datap, int nbytes) {
    debug("[%d:      ] Write data (fd = %d, nbytes = %d)", getpid(), fileno(out), nbytes);
    for(int i = 0; i < nbytes; i++) {
        //debug("[%d] write 0x%x", getpid(), *datap);
        if(fputc(*datap++, out) == EOF)
            return EOF;
    }
    return nbytes;
}

void assert_proper_exit_status(int err, int status) {
    cr_assert_eq(err, 0, "The test driver returned an error (%d)\n", err);
    cr_assert_eq(status, 0, "The program did not exit normally (status = 0x%x)\n", status);
}

void assert_worker_state_change(EVENT *ep, int *env, void *args) {
    long state = (long)args;
    WORKER *worker = find_worker(ep->pid);
    cr_assert(worker != NULL, "Worker (pid %d) does not exist", ep->pid);
    cr_assert_eq(worker_state(worker), state,
        "Worker %d state change is not the expected (Got: %s, Expected: %s)",
        worker_state_names[state], worker_state_names[state]);
}

void assert_num_workers(EVENT *ep, int *env, void *args) {
    long num = (long)args;
    cr_assert_eq(num, num_workers,
    "Number of workers is not expected (Got: %d, Expected: %ld)\n", num_workers, num);
}
