#include <stdlib.h>
#include <unistd.h>

#include "client_registry.h"
#include "exchange.h"
#include "account.h"
#include "trader.h"
#include "debug.h"
#include "server.h"
#include "csapp.h"

extern EXCHANGE *exchange;
extern CLIENT_REGISTRY *client_registry;

static void terminate(int status);

void sighup_handler(int sign){
    terminate(EXIT_SUCCESS);
    free(exchange);
    free(client_registry);
}

/*
 * "Bourse" exchange server.
 *
 * Usage: bourse <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    int option;
    char *port=0;
    while((option = getopt(argc, argv, "p:")) != EOF) {
        switch(option) {
            case 'p':
                port=optarg;
                break;
            default:
                printf("%s\n", "bin/bourse -p <port>");
                exit(EXIT_FAILURE);
        }
    }

    if(option == 0){
        fprintf(stderr, "Not enter option!");
        terminate(EXIT_FAILURE);
    }

    // Perform required initializations of the client_registry,
    // maze, and player modules.
    client_registry = creg_init();
    accounts_init();
    traders_init();
    exchange = exchange_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function brs_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.
    struct sigaction sign;
    sign.sa_handler = &sighup_handler;
    sigemptyset(&sign.sa_mask);
    sign.sa_flags = 0;
    if(sigaction(SIGHUP, &sign, NULL)==-1){
        terminate(EXIT_FAILURE);
    }

    int listenfd;
    int *connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    listenfd = Open_listenfd(port);
    while (1) {
        connfd = (int *)Malloc(sizeof(int));
        clientlen = sizeof(struct sockaddr_storage);
        *connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Pthread_create(&tid, NULL, brs_client_service, connfd);
    }

    fprintf(stderr, "You have to finish implementing main() "
	    "before the Bourse server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    exchange_fini(exchange);
    traders_fini();
    accounts_fini();

    debug("Bourse server terminating");
    exit(status);
}
