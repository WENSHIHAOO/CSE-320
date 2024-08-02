#include <criterion/criterion.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "debug.h"
#include "protocol.h"
#include "excludes.h"

#define SERVER_PORT 9999
#define SERVER_HOSTNAME "localhost"

/*
 * Get the server address.
 * This is done once, so we don't have to worry about the library
 * functions being thread safe.
 */
static struct in_addr server_addr;

static int get_server_address(void) {
    struct hostent *he;
    if((he = gethostbyname(SERVER_HOSTNAME)) == NULL) {
	perror("gethostbyname");
	return -1;
    }
    memcpy(&server_addr, he->h_addr, sizeof(server_addr));
    return 0;
}

/*
 * Connect to the server.
 *
 * Returns: connection file descriptor in case of success.
 * Returns -1 and sets errno in case of error.
 */
static int proto_connect(void) {
    struct sockaddr_in sa;
    int sfd;

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	return(-1);
    }
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(SERVER_PORT);
    memcpy(&sa.sin_addr.s_addr, &server_addr, sizeof(sa));
    if(connect(sfd, (struct sockaddr *)(&sa), sizeof(sa)) < 0) {
	perror("connect");
	close(sfd);
	return(-1);
    }
    return sfd;
}

/*
 * This should be a test of the server at the client interface level.
 * The basic idea is to feed the client command lines and analyze the responses.
 *
 * WARNING: These tests are coordinated to all run concurrently.
 * You must use --jobs XXX where XXX is sufficiently large to allow them all to
 * run, otherwise there will be issues.  The sleep times in the tests also have
 * to be adjusted if any changes are made.
 */

static void init() {
#ifndef NO_SERVER
    int ret;
    int i = 0;
    do { // Wait for server to start
	ret = system("netstat -an | fgrep '0.0.0.0:9999' > /dev/null");
	sleep(1);
    } while(++i < 30 && WEXITSTATUS(ret));
#endif
}

static void fini() {
}

/*
 * Thread to run a command using system() and collect the exit status.
 */
static void *system_thread(void *arg) {
    long ret = system((char *)arg);
    return (void *)ret;
}

// Criterion seems to sort tests by name.  This one can't be delayed
// or others will time out.
Test(server_suite, 00_start_server, .timeout = 60) {
#ifdef NO_SERVER
    cr_assert_fail("Server was not implemented");
#endif
    fprintf(stderr, "server_suite/00_start_server\n");
    int server_pid = 0;
    int ret = system("netstat -an | fgrep '0.0.0.0:9999' > /dev/null");
    cr_assert_neq(WEXITSTATUS(ret), 0, "Server was already running");
    fprintf(stderr, "Starting server...");
    if((server_pid = fork()) == 0) {
	execlp("valgrind", "bourse", "--leak-check=full", "--track-fds=yes",
	       "--error-exitcode=37", "--log-file=valgrind.out", "bin/bourse", "-p", "9999", NULL);
	fprintf(stderr, "Failed to exec server\n");
	abort();
    }
    fprintf(stderr, "pid = %d\n", server_pid);
    char *cmd = "sleep 20";
    pthread_t tid;
    pthread_create(&tid, NULL, system_thread, cmd);
    pthread_join(tid, NULL);
    cr_assert_neq(server_pid, 0, "Server was not started by this test");
    fprintf(stderr, "Sending SIGHUP to server pid %d\n", server_pid);
    kill(server_pid, SIGHUP);
    sleep(5);
    kill(server_pid, SIGKILL);
    wait(&ret);
    fprintf(stderr, "Server wait() returned = 0x%x\n", ret);
    if(WIFSIGNALED(ret)) {
	fprintf(stderr, "Server terminated with signal %d\n", WTERMSIG(ret));	
	system("cat valgrind.out");
	if(WTERMSIG(ret) == 9)
	    cr_assert_fail("Server did not terminate after SIGHUP");
    }
    if(WEXITSTATUS(ret) == 37)
	system("cat valgrind.out");
    cr_assert_neq(WEXITSTATUS(ret), 37, "Valgrind reported errors");
    cr_assert_eq(WEXITSTATUS(ret), 0, "Server exit status was not 0");
}

Test(server_suite, 01_connect_disconnect, .init = init, .fini = fini, .timeout = 5) {
#ifdef NO_SERVER
    cr_assert_fail("Server was not implemented");
#endif
    int ret = system("util/client -p 9999 </dev/null | grep 'Connected to server'");
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
}

Test(server_suite, 02_login, .init = init, .fini = fini, .timeout = 5) {
#ifdef NO_SERVER
    cr_assert_fail("Server was not implemented");
#endif
    // The client program terminates due to EOF on the input immediately after the expected
    // ACK packet is received.
    int ret = system("(echo 'login Alice_02' | util/client -q -p 9999) 2>&1 > 02_login.out");
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
    ret = system("grep 'EOF on stdin' 02_login.out");
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
}

Test(server_suite, 03_login_status, .init = init, .fini = fini, .timeout = 5) {
#ifdef NO_SERVER
    cr_assert_fail("Server was not implemented");
#endif
    int ret = system("(cat tests/rsrc/03_login_status | util/client -q -p 9999 > 03_login_status.out) 2>&1");
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
    ret = system("grep 'EOF on stdin' 03_login_status.out");
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
}

Test(server_suite, 04_login_buy, .init = init, .fini = fini, .timeout = 5) {
#ifdef NO_SERVER
    cr_assert_fail("Server was not implemented");
#endif
    int ret = system("(cat tests/rsrc/04_login_buy | util/client -q -p 9999 > 04_login_buy.out) 2>&1");
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
    // A trade should occur with 05_login_sell, leaving the exchange empty of orders.
    ret = system("grep 'EOF on stdin' 04_login_buy.out");
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
}

Test(server_suite, 05_login_sell, .init = init, .fini = fini, .timeout = 5) {
#ifdef NO_SERVER
    cr_assert_fail("Server was not implemented");
#endif
    int ret = system("(cat tests/rsrc/05_login_sell | util/client -q -p 9999 > 05_login_sell.out) 2>&1");
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
    // A trade should occur with 04_login_buy, leaving the exchange empty of orders.
    ret = system("grep 'EOF on stdin' 05_login_sell.out");
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
}
