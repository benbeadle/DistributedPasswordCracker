54#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <strings.h>
#include <errno.h>
#include <string.h>

// Global Parameters. For both server and clients.

#define _EPOCH_LTH 2.0
#define _EPOCH_CNT 5;
#define _DROP_RATE 0.0;

void lsp_set_epoch_lth(double lth);
void lsp_set_epoch_cnt(int cnt);
void lsp_set_drop_rate(double rate);

class lsp_client;

typedef struct
{

	
} lsp_client;

lsp_client* lsp_client_create(const char* dest, int port);
int lsp_client_read(lsp_client* a_client, uint8_t* pld);
bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth);
bool lsp_client_close(lsp_client* a_client);


typedef struct
{

} lsp_server;


lsp_server* lsp_server_create(int port);
int  lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id);
bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, uint32_t conn_id);
bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id);

typedef struct{
	uint32_t connid;
	uint32_t seqnum;
	uint8_t payload[];
} lsp_packet;

/*
*
*
*				FUN STRUCTURES MATT MADE, BE JELLY
*				(comma requested by Ana)
*
*/
typedef struct{
	int socketfd;
	struct sockaddr_in serveraddr;          /* Internet endpoint address */
	pthread_t server_thread;
	linked_packet* inbox_head;
	linked_packet* outbox_head;
}lsp_server;

typedef struct{
	lsp_packet packet;
	linked_packet* next;
}linked_packet;

//Creating a node
void add_packet(lsp_packet newpacket, linked_packet* box){
	while(box->next != NULL){
		box = box->next;
	}
	linked_packet* next_linked_packet = malloc(sizeof(linked_packet));
	next_linked_packet->packet = newpacket;
	box->next = next_linked_packet;
}

//add to the front of linked list
void add_packet_front(lsp_packet newpacket, linked_packet* box){
	linked_packet* new_linked_packet;	
	new_linked_packet = malloc(sizeof(linked_packet));
	new_linked_packet->packet = newpacket;
	new_linked_packet->next = box->next;
	box->next = new_linked_packet;
}

//Add to the end of linked list
void add_packet_end(lsp_packet newpacket, linked_packet* box){
	linked_packet* current = box;
	linked_packet* new_linked_packet;
	new_linked_packet = malloc(sizeof(linked_packet));
	if(new_linked_packet == NULL){
		printf("malloc failed\n");
		exit(-1);
	}
	new_linked_packet->packet = newpacket;
	new_linked_packet->next = NULL;

	while(current->next){
		current = current->next;
	}
	current->next = new_linked_packet;
}

//Delete from the front
lsp_packet consume_packet(linked_packet* box){
	if(box == NULL){ return NULL }
	linked_packet* current_packet = box;
	lsp_packet packet = current_packet->packet;
	if(box->next != NULL){
		box = box->next;
	} else {
		box = NULL;
	}
	free(current_packet);
	return packet;
}