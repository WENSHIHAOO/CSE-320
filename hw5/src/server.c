#include "server.h"
#include "csapp.h"

void *brs_client_service(void *arg){
	int connfd=*(int *)arg;
	free(arg);
	pthread_detach(pthread_self());
	if(creg_register(client_registry, connfd) == -1){
		return NULL;
	}

	BRS_PACKET_HEADER hdr;
	void *payloadp=NULL;
	TRADER *trader=NULL;
	ACCOUNT*account=NULL;
	BRS_STATUS_INFO *status = malloc(sizeof(BRS_STATUS_INFO));
	while(proto_recv_packet(connfd, &hdr, &payloadp) != -1){
		if(hdr.type == BRS_LOGIN_PKT){
			((char *)payloadp)[ntohs(hdr.size)]=0;
			if((trader=trader_login(connfd, payloadp)) == NULL){
				hdr.type=BRS_NACK_PKT;
				hdr.size=0;
				proto_send_packet(connfd, &hdr, NULL);
			}
			else{
				hdr.type=BRS_ACK_PKT;
				hdr.size=0;
				proto_send_packet(connfd, &hdr, NULL);
			}
		}

		if(trader == NULL){
			hdr.type=BRS_NACK_PKT;
			hdr.size=0;
			proto_send_packet(connfd, &hdr, NULL);
		}
		else{
			account=trader_get_account(trader);
		    status->orderid = 0;
		    status->quantity = 0;

		    if(hdr.type == BRS_STATUS_PKT){
		    	exchange_get_status(exchange, account, status);
			    hdr.type=BRS_ACK_PKT;
				hdr.size=sizeof(status);
				proto_send_packet(connfd, &hdr, &status);
			}
			else if(hdr.type == BRS_DEPOSIT_PKT){
				account_increase_balance(account, ntohl(((BRS_FUNDS_INFO *)payloadp)->amount));
				exchange_get_status(exchange, account, status);

				//fprintf(stderr, "deposit: in %u, total %u", ((BRS_FUNDS_INFO *)payloadp)->amount, status->balance);

			    hdr.type=BRS_ACK_PKT;
				hdr.size=sizeof(status);
				proto_send_packet(connfd, &hdr, &status);
			}
			else if(hdr.type == BRS_WITHDRAW_PKT){
				if(account_decrease_balance(account, ntohl(((BRS_FUNDS_INFO *)payloadp)->amount)) == 0){
					exchange_get_status(exchange, account, status);

					//fprintf(stderr, "withdaw: de %u, total %u", ((BRS_FUNDS_INFO *)payloadp)->amount, status->balance);

				    hdr.type=BRS_ACK_PKT;
					hdr.size=sizeof(status);
					proto_send_packet(connfd, &hdr, &status);
				}
				else{
					hdr.type=BRS_NACK_PKT;
					hdr.size=0;
					proto_send_packet(connfd, &hdr, NULL);
				}
			}
			else if(hdr.type == BRS_ESCROW_PKT){
				account_increase_inventory(account, ntohl(((BRS_ESCROW_INFO *)payloadp)->quantity));
				exchange_get_status(exchange, account, status);

				//fprintf(stderr, "escrow: in %u, total %u", ((BRS_ESCROW_INFO *)payloadp)->quantity, status->inventory);

			    hdr.type=BRS_ACK_PKT;
				hdr.size=sizeof(status);
				proto_send_packet(connfd, &hdr, &status);

			}
			else if(hdr.type == BRS_RELEASE_PKT){
				if(account_decrease_inventory(account, ntohl(((BRS_ESCROW_INFO *)payloadp)->quantity)) == 0){
					exchange_get_status(exchange, account, status);

					//fprintf(stderr, "escrow: de %u, total %u", ((BRS_ESCROW_INFO *)payloadp)->quantity, status->inventory);

				    hdr.type=BRS_ACK_PKT;
					hdr.size=sizeof(status);
					proto_send_packet(connfd, &hdr, &status);
				}
				else{
					hdr.type=BRS_NACK_PKT;
					hdr.size=0;
					proto_send_packet(connfd, &hdr, NULL);
				}

			}
			else if(hdr.type == BRS_BUY_PKT){
				if((exchange_post_buy(exchange, trader, ntohl(((BRS_ORDER_INFO *)payloadp)->quantity), ntohl(((BRS_ORDER_INFO *)payloadp)->price)))==0){
					hdr.type=BRS_NACK_PKT;
					hdr.size=0;
					proto_send_packet(connfd, &hdr, NULL);
				}
			}
			else if(hdr.type == BRS_SELL_PKT){
				if((exchange_post_sell(exchange, trader, ntohl(((BRS_ORDER_INFO *)payloadp)->quantity), ntohl(((BRS_ORDER_INFO *)payloadp)->price)))==0){
					hdr.type=BRS_NACK_PKT;
					hdr.size=0;
					proto_send_packet(connfd, &hdr, NULL);
				}
			}
			else if(hdr.type == BRS_CANCEL_PKT){
				quantity_t quantity;
				if(exchange_cancel(exchange, trader, ntohl(((BRS_CANCEL_INFO *)payloadp)->order),&quantity) == -1){
					hdr.type=BRS_NACK_PKT;
					hdr.size=0;
					proto_send_packet(connfd, &hdr, NULL);
				}
			}
		}

		if(payloadp != NULL){
			free(payloadp);
			payloadp = NULL;
		}
	}
	free(status);
	if(trader != NULL){
		trader_logout(trader);
	}
	creg_unregister(client_registry, connfd);
	return NULL;
}