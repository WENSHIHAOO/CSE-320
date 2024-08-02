#include <criterion/criterion.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include "debug.h"
#include "protocol.h"
#include "trader.h"
#include "excludes.h"

/* Number of threads we create in multithreaded tests. */
#define NTHREAD (10)

/* Number of iterations we use in several tests. */
#define NITER (10000)

static int nullfd;
static int filefd;

static void init() {
    if((nullfd = open("/dev/null", O_WRONLY, 0777)) < 0) {
	printf("Open failed\n");
	abort();
    }
    accounts_init();
    traders_init();
}

#define PACKET_FILE "packet.out"

static void init_file(char *name) {
    if((filefd = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777)) < 0) {
	printf("Open failed\n");
	abort();
    }
}

Test(trader_suite, trader_login_logout, .init = init, .timeout = 5) {
#ifdef NO_TRADER
    cr_assert_fail("Trader module was not implemented");
#endif
    TRADER *tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL trader pointer");
    trader_logout(tp);
}

#if 0
// These are redundant with account_suite.
Test(trader_suite, trader_balance, .init = init, .timeout = 5) {
#ifdef NO_TRADER
    cr_assert_fail("Trader module was not implemented");
#endif
    TRADER *tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL trader pointer");
    trader_increase_balance(tp, 100);
    int x = trader_decrease_balance(tp, 100);
    cr_assert_eq(x, 0, "The returned value was not 0 when it should have been");
    x = trader_decrease_balance(tp, 1);
    cr_assert_eq(x, -1, "The returned value was not -1 when it should have been");
}

Test(trader_suite, trader_balance_persist, .init = init, .timeout = 5) {
#ifdef NO_TRADER
    cr_assert_fail("Trader module was not implemented");
#endif
    TRADER *tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL pointer");
    trader_increase_balance(tp, 100);
    trader_logout(tp);
    tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL pointer");
    int x = trader_decrease_balance(tp, 100);
    cr_assert_eq(x, 0, "The returned value was not 0 when it should have been");
    x = trader_decrease_balance(tp, 1);
    cr_assert_eq(x, -1, "The returned value was not -1 when it should have been");
}

Test(trader_suite, trader_inventory, .init = init, .timeout = 5) {
#ifdef NO_TRADER
    cr_assert_fail("Trader module was not implemented");
#endif
    TRADER *tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL pointer");
    trader_increase_inventory(tp, 100);
    int x = trader_decrease_inventory(tp, 100);
    cr_assert_eq(x, 0, "The returned value was not 0 when it should have been");
    x = trader_decrease_inventory(tp, 1);
    cr_assert_eq(x, -1, "The returned value was not -1 when it should have been");
}

Test(trader_suite, trader_inventory_persist, .init = init, .timeout = 5) {
#ifdef NO_TRADER
    cr_assert_fail("Trader module was not implemented");
#endif
    TRADER *tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL pointer");
    trader_increase_inventory(tp, 100);
    trader_logout(tp);
    tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL pointer");
    int x = trader_decrease_inventory(tp, 100);
    cr_assert_eq(x, 0, "The returned value was not 0 when it should have been");
    x = trader_decrease_inventory(tp, 1);
    cr_assert_eq(x, -1, "The returned value was not -1 when it should have been");
}
#endif

Test(trader_suite, trader_send_packet, .init = init, .timeout = 5) {
    int fd;
#ifdef NO_TRADER
    cr_assert_fail("Trader module was not implemented");
#endif
    fd = open("pkt_trader_send", O_CREAT|O_TRUNC|O_RDWR, 0644);
    cr_assert(fd > 0, "Failed to create output file");
    TRADER *tp = trader_login(fd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL payload pointer");

    BRS_STATUS_INFO payload = {0};
    payload.balance = 0x11223344;
    payload.inventory = 0x11223344;
    payload.bid = 0x11223344;
    payload.ask = 0x11223344;
    payload.last = 0x11223344;
    payload.orderid = 0x11223344;
    payload.quantity = 0x11223344;
    BRS_PACKET_HEADER pkt = {0};
    pkt.type = BRS_ACK_PKT;
    pkt.size = htons(sizeof(payload));
    pkt.timestamp_sec = htonl(0x11223344);
    pkt.timestamp_nsec = htonl(0x55667788);

    int ret = trader_send_packet(tp, &pkt, &payload);
    cr_assert_eq(ret, 0, "Returned value %d was not 0", ret);
    close(fd);

    ret = system("cmp pkt_trader_send tests/rsrc/pkt_ack_with_payload");
    cr_assert_eq(ret, 0, "Packet sent did not match expected");
}

Test(trader_suite, trader_send_ack, .init = init, .timeout = 5) {
    int fd;
#ifdef NO_TRADER
    cr_assert_fail("Trader module was not implemented");
#endif
    fd = open("pkt_trader_send_ack", O_CREAT|O_TRUNC|O_RDWR, 0644);
    cr_assert(fd > 0, "Failed to create output file");
    TRADER *tp = trader_login(fd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL trader pointer");

    int ret = trader_send_ack(tp, NULL);
    cr_assert_eq(ret, 0, "Returned value %d was not 0", ret);
    close(fd);

    // We can't compare the whole packet because the timestamps will be different.
    // Just compare the first few bytes.
    ret = system("cmp -n 4 pkt_trader_send_ack tests/rsrc/pkt_ack_no_payload");
    cr_assert_eq(ret, 0, "Packet sent did not match expected");
}

Test(trader_suite, trader_send_nack, .init = init, .timeout = 5) {
    int fd;
#ifdef NO_TRADER
    cr_assert_fail("Trader module was not implemented");
#endif
    fd = open("pkt_trader_send_nack", O_CREAT|O_TRUNC|O_RDWR, 0644);
    cr_assert(fd > 0, "Failed to create output file");
    TRADER *tp = trader_login(fd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL trader pointer");

    int ret = trader_send_nack(tp);
    cr_assert_eq(ret, 0, "Returned value %d was not 0", ret);
    close(fd);

    // We can't compare the whole packet because the timestamps will be different.
    // Just compare the first few bytes.
    ret = system("cmp -n 4 pkt_trader_send_nack tests/rsrc/pkt_nack");
    cr_assert_eq(ret, 0, "Packet sent did not match expected");
}

Test(trader_suite, trader_broadcast, .init = init, .timeout = 5) {
    int alice_fd, bob_fd;
#ifdef NO_TRADER
    cr_assert_fail("Trader module was not implemented");
#endif
    alice_fd = open("pkt_trader_broadcast_alice", O_CREAT|O_TRUNC|O_RDWR, 0644);
    cr_assert(alice_fd > 0, "Failed to create output file");
    TRADER *alice = trader_login(alice_fd, "Alice");
    cr_assert_not_null(alice, "Expected non-NULL trader pointer");
    bob_fd = open("pkt_trader_broadcast_bob", O_CREAT|O_TRUNC|O_RDWR, 0644);
    cr_assert(bob_fd > 0, "Failed to create output file");
    TRADER *bob = trader_login(bob_fd, "Bob");
    cr_assert_not_null(bob, "Expected non-NULL trader pointer");

    BRS_PACKET_HEADER pkt = {0};
    pkt.type = BRS_POSTED_PKT;
    pkt.timestamp_sec = htonl(0x11223344);
    pkt.timestamp_nsec = htonl(0x55667788);

    int ret = trader_broadcast_packet(&pkt, NULL);
    cr_assert_eq(ret, 0, "Returned value %d was not 0", ret);
    close(alice_fd);
    close(bob_fd);

    ret = system("cmp pkt_trader_broadcast_alice tests/rsrc/pkt_posted");
    cr_assert_eq(ret, 0, "Packet sent did not match expected");
    ret = system("cmp pkt_trader_broadcast_bob tests/rsrc/pkt_posted");
    cr_assert_eq(ret, 0, "Packet sent did not match expected");
}

/*
 * Concurrency stress test: intended to exercise synchronization on the traders map
 * and individual trader objects:
 * Threads repeatedly run login/decrease balance/logout, then terminate.
 * Each thread delays at the start of the test, to make it more likely
 * that other threads started at about the same time are active.
 */
struct login_logout_stress_args {
    char name[2];
    int iters;
    int start_delay;
};

static void login_logout_stress(struct login_logout_stress_args *argp) {
    int fd;
    if((fd = open("/dev/null", O_WRONLY, 0777)) < 0) {
	printf("Open failed for '%s'\n", argp->name);
	abort();
    }
    TRADER *tp = trader_login(fd, argp->name);
    if(tp == NULL)
	cr_assert_fail("Initial login failed for '%s'", argp->name);
    ACCOUNT *ap = trader_get_account(tp);
    if(ap == NULL)
	cr_assert_fail("Failed to get account for '%s'", argp->name);
    account_increase_balance(ap, argp->iters);
    trader_logout(tp);
    while(argp->iters--) {
	TRADER *tp = trader_login(fd, argp->name);
	if(tp == NULL)
	    cr_assert_fail("Login failed for '%s' at iteration %d",
			   argp->name, argp->iters);
	int x = account_decrease_balance(ap, 1);
	cr_assert_eq(x, 0, "Decrease balance failed for '%s' at iteration %d",
		     argp->name, argp->iters);
	trader_logout(tp);
    }
}

static void *login_logout_stress_thread(void *arg) {
    struct login_logout_stress_args *ap = arg;
    if(ap->start_delay)
	sleep(ap->start_delay);
    login_logout_stress(ap);
    return NULL;
}

Test(trader_suite, login_logout_stress, .init = init, .timeout = 20) {
#ifdef NO_TRADER
    cr_assert_fail("Trader module was not implemented");
#endif
    // Spawn threads to run login/logout.
    pthread_t tid[NTHREAD];
    for(int i = 0; i < NTHREAD; i++) {
	struct login_logout_stress_args *ap = calloc(1, sizeof(struct login_logout_stress_args));
	ap->name[0] = 'A' + i;
	ap->name[1] = '\0';
	ap->start_delay = 1;
	ap->iters = NITER;
	pthread_create(&tid[i], NULL, login_logout_stress_thread, ap);
    }

    // Wait for all threads to finish.
    for(int i = 0; i < NTHREAD; i++)
	pthread_join(tid[i], NULL);

    // The test is deemed successful if it completes without crashing, deadlocking,
    // or having any of the logins fail along the way.
}
