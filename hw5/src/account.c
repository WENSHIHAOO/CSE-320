#include "account.h"
#include "csapp.h"



typedef struct account{
	char *name;
	funds_t last;
	funds_t balance;
	quantity_t inventory;
}ACCOUNT;

typedef struct accounts{
	sem_t mutex;/* Protects accesses to accounts */
	//sem_t slots;/* Counts available slots */
    //sem_t items;/* Counts available items */
	ACCOUNT *AC[MAX_ACCOUNTS];
}ACCOUNTS;

ACCOUNTS *ACCOUNTSR;

int accounts_init(void){
	ACCOUNTSR = (ACCOUNTS *)malloc(sizeof(ACCOUNTS));
	if(ACCOUNTSR==NULL){
		return -1;
	}
	for(int i=0; i<MAX_ACCOUNTS; i++){
		ACCOUNTSR->AC[i]=NULL;
	}
	Sem_init(&ACCOUNTSR->mutex, 0, 1);				/* Binary semaphore for locking */
    //Sem_init(&ACCOUNTSR->slots, 0, MAX_ACCOUNTS);	/* Initially, clients has n empty slots */
    //Sem_init(&ACCOUNTSR->items, 0, 0);				/* Initially, clients has zero data items */
	return 0;
}


void accounts_fini(void){
	int i=0;
	while(i<MAX_ACCOUNTS && ACCOUNTSR->AC[i] != NULL){
		free(ACCOUNTSR->AC[i]);
		i++;
	}
	free(ACCOUNTSR);
}


ACCOUNT *account_lookup(char *name){
	P(&ACCOUNTSR->mutex);	/* Lock the buffer */
	int i=0;
	while(i<MAX_ACCOUNTS && ACCOUNTSR->AC[i] != NULL){
		if(strcmp(ACCOUNTSR->AC[i]->name, name)==0){
			V(&ACCOUNTSR->mutex);	/* Unlock the buffer */
			//fprintf(stderr, "\nlooup1: id %d name %s\n", i, ACCOUNTSR->AC[i]->name);
			return ACCOUNTSR->AC[i];
		}
		i++;
	}

	if(i<MAX_ACCOUNTS){
		ACCOUNT *A = malloc(sizeof(ACCOUNT));
		A->name = name;
		A->last=0;
		A->balance = 0;
		A->inventory = 0;
		ACCOUNTSR->AC[i]=A;
		V(&ACCOUNTSR->mutex);	/* Unlock the buffer */
		//fprintf(stderr, "\nlooup2: id %d name %s\n", i, ACCOUNTSR->AC[i]->name);
		return A;
	}
	V(&ACCOUNTSR->mutex);	/* Unlock the buffer */
	return NULL;
}



void account_increase_balance(ACCOUNT *account, funds_t amount){
	P(&ACCOUNTSR->mutex);	/* Lock the buffer */
	int i=0;
	while(i<MAX_ACCOUNTS && ACCOUNTSR->AC[i] != NULL){
		if(ACCOUNTSR->AC[i] == account){
			ACCOUNTSR->AC[i]->balance+=amount;
			break;
		}
		i++;
	}
	V(&ACCOUNTSR->mutex);	/* Unlock the buffer */
	//fprintf(stderr, "\nin_balance: id %d name %s, ba %d\n",i, ACCOUNTSR->AC[i]->name, ACCOUNTSR->AC[i]->balance);
}



int account_decrease_balance(ACCOUNT *account, funds_t amount){
	P(&ACCOUNTSR->mutex);	/* Lock the buffer */
	int i=0;
	while(i<MAX_ACCOUNTS && ACCOUNTSR->AC[i] != NULL){
		if(ACCOUNTSR->AC[i] == account){
			if(ACCOUNTSR->AC[i]->balance < amount){
				//fprintf(stderr, "\nde_balance1: id %d name %s, ba %d\n", i,ACCOUNTSR->AC[i]->name, ACCOUNTSR->AC[i]->balance);
				V(&ACCOUNTSR->mutex);	/* Unlock the buffer */
				return -1;
			}
			ACCOUNTSR->AC[i]->balance-=amount;
			break;
		}
		i++;
	}
	//fprintf(stderr, "\nde_balance2: id %d name %s, ba %d\n", i,ACCOUNTSR->AC[i]->name, ACCOUNTSR->AC[i]->balance);
	V(&ACCOUNTSR->mutex);	/* Unlock the buffer */
	return 0;
}

void account_increase_inventory(ACCOUNT *account, quantity_t quantity){
	P(&ACCOUNTSR->mutex);	/* Lock the buffer */
	int i=0;
	while(i<MAX_ACCOUNTS && ACCOUNTSR->AC[i] != NULL){
		if(ACCOUNTSR->AC[i] == account){
			ACCOUNTSR->AC[i]->inventory+=quantity;
			break;
		}
		i++;
	}
	//fprintf(stderr, "\nin_inventory: id %d name %s, in %d\n", i,ACCOUNTSR->AC[i]->name, ACCOUNTSR->AC[i]->inventory);
	V(&ACCOUNTSR->mutex);	/* Unlock the buffer */
}

int account_decrease_inventory(ACCOUNT *account, quantity_t quantity){
	P(&ACCOUNTSR->mutex);	/* Lock the buffer */
	int i=0;
	while(i<MAX_ACCOUNTS && ACCOUNTSR->AC[i] != NULL){
		if(ACCOUNTSR->AC[i] == account){
			if(ACCOUNTSR->AC[i]->inventory < quantity){
				//fprintf(stderr, "\nde_inventory1: id %d name %s, in %d\n", i,ACCOUNTSR->AC[i]->name, ACCOUNTSR->AC[i]->inventory);
				V(&ACCOUNTSR->mutex);	/* Unlock the buffer */
				return -1;
			}
			ACCOUNTSR->AC[i]->inventory-=quantity;
			break;
		}
		i++;
	}
	//fprintf(stderr, "\nde_inventory2: id %d name %s, in %d\n", i,ACCOUNTSR->AC[i]->name, ACCOUNTSR->AC[i]->inventory);
	V(&ACCOUNTSR->mutex);	/* Unlock the buffer */
	return 0;
}

void account_get_status(ACCOUNT *account, BRS_STATUS_INFO *infop){
	P(&ACCOUNTSR->mutex);	/* Lock the buffer */
	int i=0;
	while(i<MAX_ACCOUNTS && ACCOUNTSR->AC[i] != NULL){
		if(ACCOUNTSR->AC[i] == account){
			infop->balance = htonl(ACCOUNTSR->AC[i]->balance);
			infop->inventory = htonl(ACCOUNTSR->AC[i]->inventory);
			infop->last = htonl(ACCOUNTSR->AC[i]->last);
			break;
		}
		i++;
	}
	//fprintf(stderr, "\nstatus: id %d name %s, ba %d, in %d\n", i, ACCOUNTSR->AC[i]->name, ACCOUNTSR->AC[i]->balance, ACCOUNTSR->AC[i]->inventory);
	V(&ACCOUNTSR->mutex);	/* Unlock the buffer */
}