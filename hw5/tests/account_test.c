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

Test(account_suite, account_balance, .init = init, .timeout = 5) {
#ifdef NO_ACCOUNT
    cr_assert_fail("Account module was not implemented");
#endif
    TRADER *tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL trader pointer");
    ACCOUNT *ap = trader_get_account(tp);
    cr_assert_not_null(ap, "Expected non-NULL account pointer");
    account_increase_balance(ap, 100);
    int x = account_decrease_balance(ap, 100);
    cr_assert_eq(x, 0, "The returned value was not 0 when it should have been");
    x = account_decrease_balance(ap, 1);
    cr_assert_eq(x, -1, "The returned value was not -1 when it should have been");
}

Test(account_suite, account_balance_persist, .init = init, .timeout = 5) {
#ifdef NO_ACCOUNT
    cr_assert_fail("Account module was not implemented");
#endif
    TRADER *tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL trader pointer");
    ACCOUNT *ap = trader_get_account(tp);
    cr_assert_not_null(ap, "Expected non-NULL account pointer");
    account_increase_balance(ap, 100);
    trader_logout(tp);
    tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL trader pointer");
    ap = trader_get_account(tp);
    cr_assert_not_null(ap, "Expected non-NULL account pointer");
    int x = account_decrease_balance(ap, 100);
    cr_assert_eq(x, 0, "The returned value was not 0 when it should have been");
    x = account_decrease_balance(ap, 1);
    cr_assert_eq(x, -1, "The returned value was not -1 when it should have been");
}

Test(account_suite, account_inventory, .init = init, .timeout = 5) {
#ifdef NO_ACCOUNT
    cr_assert_fail("Account module was not implemented");
#endif
    TRADER *tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL trader pointer");
    ACCOUNT *ap = trader_get_account(tp);
    cr_assert_not_null(ap, "Expected non-NULL account pointer");
    account_increase_inventory(ap, 100);
    int x = account_decrease_inventory(ap, 100);
    cr_assert_eq(x, 0, "The returned value was not 0 when it should have been");
    x = account_decrease_inventory(ap, 1);
    cr_assert_eq(x, -1, "The returned value was not -1 when it should have been");
}

Test(account_suite, account_inventory_persist, .init = init, .timeout = 5) {
#ifdef NO_ACCOUNT
    cr_assert_fail("Account module was not implemented");
#endif
    TRADER *tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL trader pointer");
    ACCOUNT *ap = trader_get_account(tp);
    cr_assert_not_null(ap, "Expected non-NULL account pointer");
    account_increase_inventory(ap, 100);
    trader_logout(tp);
    tp = trader_login(nullfd, "Alice");
    cr_assert_not_null(tp, "Expected non-NULL trader pointer");
    ap = trader_get_account(tp);
    cr_assert_not_null(ap, "Expected non-NULL account pointer");
    int x = account_decrease_inventory(ap, 100);
    cr_assert_eq(x, 0, "The returned value was not 0 when it should have been");
    x = account_decrease_inventory(ap, 1);
    cr_assert_eq(x, -1, "The returned value was not -1 when it should have been");
}

// TODO: account_get_status()

