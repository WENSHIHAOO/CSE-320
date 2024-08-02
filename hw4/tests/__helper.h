#ifndef __HELPER_H
#define __HELPER_H

void assert_proper_exit_status(int err, int status);
void assert_worker_status_change(EVENT *ep, int *env, void *args);
void assert_num_workers(EVENT *ep, int *env, void *args);
int read_data_t(FILE *in, char *resultp, int nbytes);
int write_data_t(FILE *out, char *datap, int nbytes);

#endif
