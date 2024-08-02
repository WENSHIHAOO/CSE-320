#include "client_registry.h"
#include "csapp.h"


typedef struct client{
	int fd;
}CLIENT;


typedef struct client_registry{
	int count;	/* number of clients */
	sem_t mutex;/* Protects accesses to clients */
	//sem_t slots;/* Counts available slots */
    //sem_t items;/* Counts available items */
	sem_t empty;/* count=0 */
	CLIENT **clients;
	int clients_size;
}CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init(){
	CLIENT_REGISTRY *CR = (CLIENT_REGISTRY *)malloc(sizeof(CLIENT_REGISTRY));
	if(CR==NULL){
		return NULL;
	}
	CR->count=0;
	CR->clients_size=64;
	CR->clients=malloc(64*sizeof(CLIENT*));
	for(int i=0; i<CR->clients_size; i++){
		CR->clients[i]=NULL;
	}

	Sem_init(&CR->mutex, 0, 1);				/* Binary semaphore for locking */
    //Sem_init(&CR->slots, 0, MAX_TRADERS);	/* Initially, clients has n empty slots */
    //Sem_init(&CR->items, 0, 0);				/* Initially, clients has zero data items */
	Sem_init(&CR->empty, 0, 1);				/* Initially, clients is empty */

	return CR;
}

void creg_fini(CLIENT_REGISTRY *cr){
	Free(cr);
}

int creg_register(CLIENT_REGISTRY *cr, int fd){
	//P(&cr->slots);	/* Wait for available slot */
	P(&cr->mutex);	/* Lock the buffer */
	if(cr->count==cr->clients_size){
		cr->clients_size+=64;
		cr->clients=realloc(cr->clients, (cr->clients_size)*sizeof(CLIENT*));

		if(cr->clients==NULL){
			V(&cr->mutex);	/* Unlock the buffer */
			//V(&cr->items);	/* Announce available item */
			return -1;
		}

		for(int i=cr->clients_size-64; i<cr->clients_size; i++){
			cr->clients[i]=NULL;
		}
	}

	for(int i=0; i<cr->clients_size; i++){
		if(cr->clients[i]==NULL){
			CLIENT *c = malloc(sizeof(CLIENT));
			if(c==NULL){
				V(&cr->mutex);	/* Unlock the buffer */
				//V(&cr->items);	/* Announce available item */
				return -1;
			}
			c->fd=fd;
			cr->clients[i] = c;
			cr->count++;

			// fprintf(stderr, "\nregi: id %d\n", i);
			// if(i>0 && cr->clients[i-1] != NULL){
			// 	fprintf(stderr, "\nregi: id %d fd %d, last_fd %d\n", i, c->fd, cr->clients[i-1]->fd);
			// }
			break;
		}
	}

	if(cr->count==1){
		P(&cr->empty);
	}

	V(&cr->mutex);	/* Unlock the buffer */
    //V(&cr->items);	/* Announce available item */
    return 0;
}

int creg_unregister(CLIENT_REGISTRY *cr, int fd){
	//P(&cr->items);	/* Wait for available item */
	P(&cr->mutex);	/* Lock the buffer */

	int fail=0;
	for(int i=0; i<cr->clients_size; i++){
		if(cr->clients[i]!=NULL && cr->clients[i]->fd==fd){
			fail=1;
			free(cr->clients[i]);
			cr->clients[i]=NULL;
			cr->count--;

			// fprintf(stderr, "\nunregi: id %d\n", i);
			// if(i>0 && cr->clients[i-1] != NULL){
			// 	fprintf(stderr, "\nunre: id %d fd %d, last_fd %d\n", i, fd, cr->clients[i-1]->fd);
			// }
			break;
		}
	}

	if(fail==0){
		V(&cr->mutex);	/* Unlock the buffer */
    	//V(&cr->slots);	/* Announce available slot */
    	return -1;
	}

	if(cr->count==0){
		V(&cr->empty);
	}

	V(&cr->mutex);	/* Unlock the buffer */
    //V(&cr->slots);	/* Announce available slot */
    return 0;
}

void creg_wait_for_empty(CLIENT_REGISTRY *cr){
	P(&cr->empty);
	// free(*(cr->clients));
	// *(cr->clients)=NULL;
	V(&cr->empty);
}

void creg_shutdown_all(CLIENT_REGISTRY *cr){
	P(&cr->mutex);	/* Lock the buffer */

	int i=0;
	while(cr->clients[i] != NULL){
		shutdown((*(cr->clients)+i)->fd, SHUT_RD);
		free(cr->clients[i]);
	}
	cr->count=0;
	free(cr->clients);
	cr->clients=NULL;
	cr->clients_size=0;

	V(&cr->mutex);	/* Unlock the buffer */
}