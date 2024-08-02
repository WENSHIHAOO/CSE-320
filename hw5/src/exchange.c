#include "exchange.h"
#include "csapp.h"

typedef struct post{
    int buy_sell;
    TRADER *trader;
    orderid_t order;
    funds_t buy_price;
    funds_t sell_price;
    quantity_t quantity;
}POST;

typedef struct exchange{
    int count_orderid;
    int count_waitP;
    int count_buys;
    int count_sells;
    sem_t mutex;/* Protects accesses to exchange */
    sem_t wait;/* matchmaker */
    POST **waitP;
    POST **buys;
    POST **sells;
    int waitP_size;
    int buys_size;
    int sells_size;
    int high_buy;
    int low_sell;
}EXCHANGE;


void put_buy(EXCHANGE *xchg, int index){
    if(xchg->count_buys==xchg->buys_size){
        xchg->buys_size+=64;
        xchg->buys=realloc(xchg->buys, (xchg->buys_size)*sizeof(POST*));

        for(int i=xchg->buys_size-64; i<xchg->buys_size; i++){
            xchg->buys[i]=NULL;
        }
    }

    for(int i=0; i<xchg->buys_size; i++){
        if(xchg->buys[i]==NULL){
            xchg->buys[i]=xchg->waitP[index];
            xchg->waitP[index]=NULL;
            xchg->count_buys++;
            xchg->count_waitP--;

            //fprintf(stderr, "\npost_buy: id %d\n", i);
            break;
        }
    }
}

void put_sell(EXCHANGE *xchg, int index){
    if(xchg->count_sells==xchg->sells_size){
        xchg->sells_size+=64;
        xchg->sells=realloc(xchg->sells, (xchg->sells_size)*sizeof(POST*));

        for(int i=xchg->sells_size-64; i<xchg->sells_size; i++){
            xchg->sells[i]=NULL;
        }
    }

    for(int i=0; i<xchg->sells_size; i++){
        if(xchg->sells[i]==NULL){
            xchg->sells[i]=xchg->waitP[index];
            xchg->waitP[index]=NULL;
            xchg->count_sells++;
            xchg->count_waitP--;

            //fprintf(stderr, "\nput_sell: id %d\n", i);
            break;
        }
    }
}

void notify(TRADER *seller, TRADER *buyer, orderid_t seller_id, orderid_t buyer_id, quantity_t quantity, funds_t price){
    BRS_PACKET_HEADER hdr;
    hdr.size=sizeof(BRS_NOTIFY_INFO);

    BRS_NOTIFY_INFO info;
    info.buyer=buyer_id;
    info.seller=seller_id;
    info.quantity=quantity;
    info.price=price;

    hdr.type=BRS_BOUGHT_PKT;
    trader_send_packet(buyer, &hdr, &info);

    hdr.type=BRS_SOLD_PKT;
    trader_send_packet(seller, &hdr, &info);

    hdr.type=BRS_TRADED_PKT;
    trader_broadcast_packet(&hdr, &info);
}

void *matchmaker(void *arg){
    EXCHANGE *xchg=arg;
    pthread_detach(pthread_self());

    while(1){
        int fail=0;
        P(&xchg->wait);  /* Wait for available waitp */
        P(&xchg->mutex);   /* Lock the buffer */
        for(int i=0; i<xchg->waitP_size; i++){
            // find wait
            if(xchg->waitP[i] != NULL){
                if(xchg->waitP[i]->buy_sell == 0){//find waitP_buy
                    for(int j=0; j<xchg->sells_size && xchg->count_sells>0 && xchg->waitP[i]!=NULL; j++){
                        //find match
                        if(xchg->sells[j] != NULL && xchg->sells[j]->sell_price<=xchg->waitP[i]->buy_price){
                            ACCOUNT *sell_account=trader_get_account(xchg->sells[j]->trader);
                            ACCOUNT *waitP_account=trader_get_account(xchg->waitP[i]->trader);
                            uint32_t waitP_quantity=0;
                            uint32_t sell_balance=0;
                            uint32_t price_diff = xchg->waitP[i]->buy_price-xchg->sells[j]->sell_price;
                            if(xchg->sells[j]->quantity < xchg->waitP[i]->quantity){//free sell
                                waitP_quantity = xchg->sells[j]->quantity;
                                sell_balance = waitP_quantity*xchg->sells[j]->sell_price;

                                xchg->waitP[i]->quantity -= waitP_quantity;
                                notify(xchg->sells[j]->trader, xchg->waitP[i]->trader, xchg->sells[j]->order, xchg->waitP[i]->order, waitP_quantity, xchg->sells[j]->sell_price);
                                free(xchg->sells[j]);
                                xchg->sells[j]=NULL;
                                xchg->count_sells--;
                            }
                            else if(xchg->sells[j]->quantity > xchg->waitP[i]->quantity){//free waitP
                                fail=1;
                                waitP_quantity = xchg->waitP[i]->quantity;
                                sell_balance = waitP_quantity*xchg->sells[j]->sell_price;

                                xchg->sells[j]->quantity -= waitP_quantity;
                                notify(xchg->sells[j]->trader, xchg->waitP[i]->trader, xchg->sells[j]->order, xchg->waitP[i]->order, waitP_quantity, xchg->sells[j]->sell_price);
                                free(xchg->waitP[i]);
                                xchg->waitP[i]=NULL;
                                xchg->count_waitP--;
                           }
                            else{//free both
                                fail=1;
                                waitP_quantity = xchg->waitP[i]->quantity;
                                sell_balance = waitP_quantity*xchg->sells[j]->sell_price;

                                notify(xchg->sells[j]->trader, xchg->waitP[i]->trader, xchg->sells[j]->order, xchg->waitP[i]->order, waitP_quantity, xchg->sells[j]->sell_price);
                                free(xchg->sells[j]);
                                xchg->sells[j]=NULL;
                                xchg->count_sells--;
                                free(xchg->waitP[i]);
                                xchg->waitP[i]=NULL;
                                xchg->count_waitP--;
                            }

                            account_increase_balance(sell_account, sell_balance);
                            account_increase_inventory(waitP_account, waitP_quantity);
                            if(price_diff!=0){
                                account_increase_balance(waitP_account, waitP_quantity*price_diff);
                            }
                        }
                    }
                    if(fail==0){
                        put_buy(xchg, i);
                    }
                }
                else{//find waitP_sell
                    for(int j=0; j<xchg->buys_size && xchg->count_buys>0 && xchg->waitP[i]!=NULL; j++){
                        //find match
                        if(xchg->buys[j] != NULL && xchg->waitP[i]->sell_price<=xchg->buys[j]->buy_price){
                            ACCOUNT *buy_account=trader_get_account(xchg->buys[j]->trader);
                            ACCOUNT *waitP_account=trader_get_account(xchg->waitP[i]->trader);
                            uint32_t buy_quantity=0;
                            uint32_t waitP_balance=0;
                            uint32_t price_diff = xchg->buys[j]->buy_price-xchg->waitP[i]->sell_price;
                            if(xchg->buys[j]->quantity < xchg->waitP[i]->quantity){//free buy
                                buy_quantity = xchg->buys[j]->quantity;
                                waitP_balance = buy_quantity*xchg->waitP[i]->sell_price;

                                xchg->waitP[i]->quantity -= buy_quantity;
                                notify(xchg->waitP[i]->trader, xchg->buys[j]->trader, xchg->waitP[i]->order, xchg->buys[j]->order, buy_quantity, xchg->waitP[i]->sell_price);
                                free(xchg->buys[j]);
                                xchg->buys[j]=NULL;
                                xchg->count_buys--;
                            }
                            else if(xchg->buys[j]->quantity > xchg->waitP[i]->quantity){//free waitP
                                fail=1;
                                buy_quantity = xchg->waitP[i]->quantity;
                                waitP_balance = buy_quantity*xchg->waitP[i]->sell_price;

                                xchg->buys[j]->quantity -= buy_quantity;
                                notify(xchg->waitP[i]->trader, xchg->buys[j]->trader, xchg->waitP[i]->order, xchg->buys[j]->order, buy_quantity, xchg->waitP[i]->sell_price);
                                free(xchg->waitP[i]);
                                xchg->waitP[i]=NULL;
                                xchg->count_waitP--;
                            }
                            else{//free both
                                fail=1;
                                buy_quantity = xchg->waitP[i]->quantity;
                                waitP_balance = buy_quantity*xchg->waitP[i]->sell_price;

                                notify(xchg->waitP[i]->trader, xchg->buys[j]->trader, xchg->waitP[i]->order, xchg->buys[j]->order, buy_quantity, xchg->waitP[i]->sell_price);
                                free(xchg->buys[j]);
                                xchg->buys[j]=NULL;
                                xchg->count_buys--;
                                free(xchg->waitP[i]);
                                xchg->waitP[i]=NULL;
                                xchg->count_waitP--;
                            }
                            account_increase_balance(waitP_account, waitP_balance);
                            account_increase_inventory(buy_account, buy_quantity);
                            if(price_diff!=0){
                                account_increase_balance(buy_account, buy_quantity*price_diff);
                            }
                        }
                    }
                    if(fail==0){
                        put_sell(xchg, i);
                    }
                }
                break;
            }
        }
        V(&xchg->mutex);   /* unlock the buffer */
    }
    return NULL;
}

EXCHANGE *exchange_init(){
    EXCHANGE *xchg = (EXCHANGE *)malloc(sizeof(EXCHANGE));
    if(xchg==NULL){
        return NULL;
    }
    xchg->count_orderid=1;
    xchg->count_waitP=0;
    xchg->count_buys=0;
    xchg->count_sells=0;
    Sem_init(&xchg->mutex, 0, 1);             /* Binary semaphore for locking */
    Sem_init(&xchg->wait, 0, 0);             /* Initially, waitP has zero data items */

    xchg->waitP_size=64;
    xchg->waitP=malloc(64*sizeof(POST*));
    for(int i=0; i<xchg->waitP_size; i++){
        xchg->waitP[i]=NULL;
    }

    xchg->buys_size=64;
    xchg->buys=malloc(64*sizeof(POST*));
    for(int i=0; i<xchg->buys_size; i++){
        xchg->buys[i]=NULL;
    }

    xchg->sells_size=64;
    xchg->sells=malloc(64*sizeof(POST*));
    for(int i=0; i<xchg->sells_size; i++){
        xchg->sells[i]=NULL;
    }

    xchg->high_buy=0;
    xchg->low_sell=0;

    pthread_t tid;
    Pthread_create(&tid, NULL, matchmaker, xchg);
    return xchg;
}


void exchange_fini(EXCHANGE *xchg){
    for(int i=0; i<xchg->buys_size; i++){
        if(xchg->buys[i] != NULL){
            exchange_cancel(xchg, xchg->buys[i]->trader, xchg->buys[i]->order, &xchg->buys[i]->quantity);
        }
    }
    free(xchg->buys);
    xchg->count_buys=0;

    for(int i=0; i<xchg->sells_size; i++){
        if(xchg->sells[i] != NULL){
            exchange_cancel(xchg, xchg->sells[i]->trader, xchg->sells[i]->order, &xchg->sells[i]->quantity);
        }
    }
    free(xchg->sells);
    xchg->count_sells=0;
}


void exchange_get_status(EXCHANGE *xchg, ACCOUNT *account, BRS_STATUS_INFO *infop){
    infop->bid = xchg->high_buy;
    infop->ask = xchg->low_sell;
    account_get_status(account, infop);
}


orderid_t exchange_post_buy(EXCHANGE *xchg, TRADER *trader, quantity_t quantity, funds_t price){
    P(&xchg->mutex);   /* Lock the buffer */
    BRS_PACKET_HEADER hdr;
    ACCOUNT *account=trader_get_account(trader);
    if(account_decrease_balance(account, quantity*price)==-1){
        V(&xchg->mutex);   /* Unlock the buffer */
        return 0;
    }

    if(xchg->count_waitP==xchg->waitP_size){
        xchg->waitP_size+=64;
        xchg->waitP=realloc(xchg->waitP, (xchg->waitP_size)*sizeof(POST*));

        for(int i=xchg->waitP_size-64; i<xchg->waitP_size; i++){
            xchg->waitP[i]=NULL;
        }
    }

    for(int i=0; i<xchg->waitP_size; i++){
        if(xchg->waitP[i]==NULL){
            POST *p = malloc(sizeof(POST));
            p->buy_sell=0;
            p->trader=trader;
            p->order=xchg->count_orderid;
            p->buy_price=price;
            p->sell_price=0;
            p->quantity=quantity;
            xchg->waitP[i] = p;
            xchg->count_waitP++;
            xchg->count_orderid++;
            //fprintf(stderr, "\npost_buy: index %d, id %d\n", i, xchg->count_orderid-1);
            break;
        }
    }

    BRS_STATUS_INFO status;
    exchange_get_status(xchg, account, &status);
    status.orderid=htonl(xchg->count_orderid-1);
    trader_send_ack(trader, &status);

    V(&xchg->mutex);   /* Unlock the buffer */
    V(&xchg->wait);

    hdr.type=BRS_POSTED_PKT;
    hdr.size=sizeof(BRS_NOTIFY_INFO);

    BRS_NOTIFY_INFO info;
    info.buyer=xchg->count_orderid-1;
    info.seller=0;
    info.quantity=quantity;
    info.price=price;

    trader_broadcast_packet(&hdr, &info);
    return xchg->count_orderid-1;
}


orderid_t exchange_post_sell(EXCHANGE *xchg, TRADER *trader, quantity_t quantity, funds_t price){
    P(&xchg->mutex);   /* Lock the buffer */
    BRS_PACKET_HEADER hdr;
    ACCOUNT *account=trader_get_account(trader);
    if(account_decrease_inventory(account, quantity) == -1){
        V(&xchg->mutex);   /* Unlock the buffer */
        return 0;
    }

    if(xchg->count_waitP==xchg->waitP_size){
        xchg->waitP_size+=64;
        xchg->waitP=realloc(xchg->waitP, (xchg->waitP_size)*sizeof(POST*));

        for(int i=xchg->waitP_size-64; i<xchg->waitP_size; i++){
            xchg->waitP[i]=NULL;
        }
    }

    for(int i=0; i<xchg->waitP_size; i++){
        if(xchg->waitP[i]==NULL){
            POST *p = malloc(sizeof(POST));
            p->buy_sell=1;
            p->trader=trader;
            p->order=xchg->count_orderid;
            p->buy_price=0;
            p->sell_price=price;
            p->quantity=quantity;
            xchg->waitP[i] = p;
            xchg->count_waitP++;
            xchg->count_orderid++;
            //fprintf(stderr, "\npost_sell: index %d, id %d\n", i, xchg->count_orderid-1);
            break;
        }
    }

    BRS_STATUS_INFO status;
    exchange_get_status(xchg, account, &status);
    status.orderid=htonl(xchg->count_orderid-1);
    trader_send_ack(trader, &status);

    V(&xchg->mutex);   /* Unlock the buffer */
    V(&xchg->wait);

    hdr.type=BRS_POSTED_PKT;
    hdr.size=sizeof(BRS_NOTIFY_INFO);

    BRS_NOTIFY_INFO info;
    info.buyer=0;
    info.seller=xchg->count_orderid-1;
    info.quantity=quantity;
    info.price=price;

    trader_broadcast_packet(&hdr, &info);
    return xchg->count_orderid-1;
}


int exchange_cancel(EXCHANGE *xchg, TRADER *trader, orderid_t order, quantity_t *quantity){
    P(&xchg->mutex);   /* Lock the buffer */
    int fail=0;
    BRS_PACKET_HEADER hdr;
    BRS_NOTIFY_INFO info;
    info.buyer=0;
    info.seller=0;
    for(int i=0; i<xchg->buys_size; i++){
        if(xchg->buys[i] != NULL && xchg->buys[i]->trader==trader && xchg->buys[i]->order==order){
            fail=1;
            info.buyer=order;
            info.quantity=xchg->buys[i]->quantity;
            info.price=xchg->buys[i]->buy_price;
            ACCOUNT *account=trader_get_account(trader);
            account_increase_balance(account, xchg->buys[i]->quantity*xchg->buys[i]->buy_price);
            *quantity=xchg->buys[i]->quantity;
            free(xchg->buys[i]);
            xchg->buys[i]=NULL;
            xchg->count_buys--;

            BRS_STATUS_INFO status;
            exchange_get_status(xchg, account, &status);
            status.orderid=htonl(xchg->count_orderid-1);
            trader_send_ack(trader, &status);
        }
    }
    for(int i=0; i<xchg->sells_size; i++){
        if(xchg->sells[i] != NULL && xchg->sells[i]->trader==trader && xchg->sells[i]->order==order){
            fail=1;
            info.seller=order;
            info.quantity=xchg->sells[i]->quantity;
            info.price=xchg->sells[i]->sell_price;
            ACCOUNT *account=trader_get_account(trader);
            account_increase_inventory(account, xchg->sells[i]->quantity);
            *quantity=xchg->sells[i]->quantity;
            free(xchg->sells[i]);
            xchg->sells[i]=NULL;
            xchg->count_sells--;

            BRS_STATUS_INFO status;
            exchange_get_status(xchg, account, &status);
            status.orderid=htonl(xchg->count_orderid-1);
            status.quantity=htonl(*quantity);
            trader_send_ack(trader, &status);
        }
    }
    if(fail==0){
        V(&xchg->mutex);   /* Unlock the buffer */
        return -1;
    }

    hdr.type=BRS_CANCELED_PKT;
    hdr.size=sizeof(BRS_NOTIFY_INFO);

    trader_broadcast_packet(&hdr, &info);
    V(&xchg->mutex);   /* Unlock the buffer */
    return 0;
}
