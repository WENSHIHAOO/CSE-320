#include <criterion/criterion.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "debug.h"
#include "exchange.h"
#include "excludes.h"

/* Number of threads we create in multithreaded tests. */
#define NTHREAD (10)

/* Number of iterations we use in several tests. */
#define NITER (2500)

static int nullfd;

static void init(void) {
    if((nullfd = open("/dev/null", O_WRONLY, 0777)) < 0) {
	printf("Open failed\n");
	abort();
    }
    accounts_init();
    traders_init();
}

Test(exchange_suite, init_exchange, .init = init, .timeout = 5) {
#ifdef NO_EXCHANGE
    cr_assert_fail("Exchange module was not implemented");
#endif
    EXCHANGE *ex = exchange_init();
    cr_assert_not_null(ex, "The returned value was NULL");
}

Test(exchange_suite, post_buy, .init = init, .timeout = 5) {
#ifdef NO_EXCHANGE
    cr_assert_fail("Exchange module was not implemented");
#endif
    EXCHANGE *ex = exchange_init();
    TRADER *tp = trader_login(nullfd, "Alice");
    ACCOUNT *ap = trader_get_account(tp);
    account_increase_balance(ap, 100);
    funds_t bid = 5;
    orderid_t id = exchange_post_buy(ex, tp, 10, bid);
    cr_assert_neq(id, 0, "The returned order ID was 0");
    BRS_STATUS_INFO info;
    exchange_get_status(ex, ap, &info);
    if(info.bid != bid && ntohl(info.bid) != bid)
	cr_assert_fail("The returned bid value was not the expected value (%d)", bid);
}

Test(exchange_suite, post_sell, .init = init, .timeout = 5) {
#ifdef NO_EXCHANGE
    cr_assert_fail("Exchange module was not implemented");
#endif
    EXCHANGE *ex = exchange_init();
    TRADER *tp = trader_login(nullfd, "Alice");
    ACCOUNT *ap = trader_get_account(tp);
    account_increase_inventory(ap, 100);
    funds_t ask = 5;
    orderid_t id = exchange_post_sell(ex, tp, 10, ask);
    cr_assert_neq(id, 0, "The returned order ID was 0");
    BRS_STATUS_INFO info;
    exchange_get_status(ex, ap, &info);
    if(info.ask != ask && ntohl(info.ask) != ask)
	cr_assert_fail("The returned ask value was not the expected value (%d)", ask);
}

Test(exchange_suite, self_trade, .init = init, .timeout = 5) {
#ifdef NO_EXCHANGE
    cr_assert_fail("Exchange module was not implemented");
#endif
    EXCHANGE *ex = exchange_init();
    TRADER *tp = trader_login(nullfd, "Alice");
    ACCOUNT *ap = trader_get_account(tp);
    account_increase_balance(ap, 100);
    account_increase_inventory(ap, 100);
    funds_t price = 5;
    orderid_t buy_id = exchange_post_buy(ex, tp, 10, price);
    cr_assert_neq(buy_id, 0, "The returned order ID was 0");
    orderid_t sell_id = exchange_post_sell(ex, tp, 10, price);
    cr_assert_neq(sell_id, 0, "The returned order ID was 0");
    sleep(1);  // Give the matchmaker a chance to work
    BRS_STATUS_INFO info;
    exchange_get_status(ex, ap, &info);
    if(info.last != price && ntohl(info.last) != price)
	cr_assert_fail("The returned last trade price was not the expected price (%d)",
		       price);
}

Test(exchange_suite, no_trade, .init = init, .timeout = 5) {
#ifdef NO_EXCHANGE
    cr_assert_fail("Exchange module was not implemented");
#endif
    EXCHANGE *ex = exchange_init();
    TRADER *tp = trader_login(nullfd, "Alice");
    ACCOUNT *ap = trader_get_account(tp);
    account_increase_balance(ap, 100);
    account_increase_inventory(ap, 100);
    funds_t bid = 5;
    funds_t ask = 6;
    orderid_t buy_id = exchange_post_buy(ex, tp, 10, bid);
    cr_assert_neq(buy_id, 0, "The returned order ID was 0");
    orderid_t sell_id = exchange_post_sell(ex, tp, 10, ask);
    cr_assert_neq(sell_id, 0, "The returned order ID was 0");
    sleep(1);  // Give the matchmaker a chance to work
    BRS_STATUS_INFO info;
    exchange_get_status(ex, ap, &info);
    if(info.last == bid || ntohl(info.last) == bid ||
       info.last == ask || ntohl(info.last) == ask)
	cr_assert_fail("The returned last trade price should not be (%d)",
		       ntohl(info.last));
}

/*
 * Concurrency stress test.
 * Several threads, all acting on behalf of the same trader, post random
 * buy and sell orders concurrently.  After a large number of orders,
 * the final balance and inventory is checked.
 */

#define BALANCE (100 * NITER)       // Initial balance
#define INVENTORY (100 * NITER)     // Initial inventory
#define MIN_BID_PRICE (1)           // Minimum bid price
#define MAX_ASK_PRICE (10)          // Maximum ask price

struct random_trade_args {
    int start_delay;    // Starting delay
    unsigned int seed;  // RNG seed
    EXCHANGE *exchange; // Exchange
    TRADER *trader;     // Trader
    int iters;          // Iterations remaining
};

static void *random_trade_thread(void *arg) {
    struct random_trade_args *ap = arg;
    if(ap->start_delay)
	sleep(ap->start_delay);
    while(ap->iters--) {
	quantity_t quantity = 1 + rand_r(&ap->seed) % 10;
	funds_t price = MIN_BID_PRICE + rand_r(&ap->seed) % (MAX_ASK_PRICE - MIN_BID_PRICE + 1);
	int sell = rand_r(&ap->seed) % 2;
	if(sell) {
	    exchange_post_sell(ap->exchange, ap->trader, quantity, price);
	} else {
	    exchange_post_buy(ap->exchange, ap->trader, quantity, price);
	}
    }
    return NULL;
}

Test(exchange_suite, random_trade_test, .init = init, .timeout = 30) {
    extern void show_exchange(EXCHANGE *ex);
#ifdef NO_EXCHANGE
    cr_assert_fail("Exchange module was not implemented");
#endif
    EXCHANGE *ex = exchange_init();
    TRADER *tp = trader_login(nullfd, "Alice");
    ACCOUNT *ap = trader_get_account(tp);
    account_increase_balance(ap, BALANCE);
    account_increase_inventory(ap, INVENTORY);

    // Spawn threads to run random trades.
    pthread_t tid[NTHREAD];
    for(int i = 0; i < NTHREAD; i++) {
	struct random_trade_args *argp = calloc(1, sizeof(struct random_trade_args));
	argp->start_delay = 1;
	argp->seed = i;
	argp->exchange = ex;
	argp->trader = tp;
	argp->iters = NITER;
	pthread_create(&tid[i], NULL, random_trade_thread, argp);
    }

    // Wait for all threads to finish.
    for(int i = 0; i < NTHREAD; i++)
	pthread_join(tid[i], NULL);

    //show_exchange(ex);
    fprintf(stderr, "\n\n***Checking balances and inventories***\n\n");

    // Add additional balance to make sure that there are enough funds to buy
    // back all the outstanding sell orders at a high price.
    account_increase_balance(ap, MAX_ASK_PRICE * INVENTORY);

    // Post a buy order at the maximum ask price, to buy up all outstanding sell orders.
    fprintf(stderr, "Posting buy order for %d at %d\n", INVENTORY, MAX_ASK_PRICE);
    exchange_post_buy(ex, tp, INVENTORY, MAX_ASK_PRICE);

    // Give the matchmaker some time to work.
    sleep(1);
    //show_exchange(ex);

    // All sell orders have been executed, so the inventory should now be the initial amount.
    int x = account_decrease_inventory(ap, INVENTORY);
    cr_assert_eq(x, 0, "The final inventory was too small");
    x = account_decrease_inventory(ap, 1);
    cr_assert_eq(x, -1, "The final inventory was too large");

    // At this point, there could be buy orders with a total price up to the amount of
    // funds which have been added, which is BALANCE + MAX_ASK_PRICE * INVENTORY.
    // These orders would be capable of purchasing a quantity totalling
    // (BALANCE + MAX_ASK_PRICE * INVENTORY) / MIN_BID_PRICE, rounded up to the nearest
    // integer.  So, post a sell order for this amount at the minimum bid price, so as to
    // execute all outstanding buy orders.
    int quantity = (BALANCE + MAX_ASK_PRICE * INVENTORY + MIN_BID_PRICE - 1) / MIN_BID_PRICE;
    account_increase_inventory(ap, quantity);
    fprintf(stderr, "Posting sell order for %d at %d\n", quantity, MIN_BID_PRICE);
    exchange_post_sell(ex, tp, quantity, MIN_BID_PRICE);

    // Give the matchmaker some time to work.
    sleep(1);

    // All buy orders have been executed, so the balance should now be the initial amount,
    // incremented by the amount added above.
    x = account_decrease_balance(ap, BALANCE + MAX_ASK_PRICE * INVENTORY);
    //if(x != 0)
    //    show_exchange(ex);
    cr_assert_eq(x, 0, "The final balance was too small (saw %d, expected %d)", x, 0);
    x = account_decrease_balance(ap, 1);
    cr_assert_eq(x, -1, "The final balance was too large (saw %d, expected %d)", x, -1);
}
