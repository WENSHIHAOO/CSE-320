#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "csapp.h"
#include "protocol.h"

int proto_send_packet(int fd, BRS_PACKET_HEADER *hdr, void *payload){
	if(write(fd, hdr, sizeof(BRS_PACKET_HEADER)) == -1){
		return -1;
	}
	else{
		uint16_t size = ntohs(hdr->size);
		if(write(fd, payload, size) == -1){
			return -1;
		}

		struct timespec *tv = malloc(sizeof(struct timespec));
		if (clock_gettime(CLOCK_REALTIME, tv) == -1) {
			return -1;
		}
		hdr->timestamp_sec = tv->tv_sec;
		hdr->timestamp_nsec = tv->tv_nsec;
		free(tv);

		//fprintf(stderr, "\nsend_size: type %u, h_size %u, size %u, sec %u, nsec %u\n", hdr->type, hdr->size, size, hdr->timestamp_sec, hdr->timestamp_nsec);
	}

	return 0;
}

uint32_t sec=0;
int proto_recv_packet(int fd, BRS_PACKET_HEADER *hdr, void **payloadp){
	hdr->type=0;
	hdr->timestamp_sec=0;
	if(read(fd, hdr, sizeof(BRS_PACKET_HEADER)) == -1){
		return -1;
	}
	else{
		if(hdr->type == BRS_NO_PKT || hdr->type > BRS_CANCEL_PKT || sec>=ntohl(hdr->timestamp_sec)){
			return -1;
		}
		sec=ntohl(hdr->timestamp_sec);

		void *payload = (void *)malloc(ntohs(hdr->size));
		if(read(fd, payload, ntohs(hdr->size)) == -1){
			free(payload);
			return -1;
		}

		if(payload == NULL){
			free(payload);
		}
		else{
			*payloadp=payload;
		}

		//fprintf(stderr, "\nrecv_size type %u, h_size %u, pay %s, sec %u, nsec %u, g_sec %u\n", hdr->type, hdr->size, (char *)payload, hdr->timestamp_sec, hdr->timestamp_nsec, sec);
	}

	return 0;
}