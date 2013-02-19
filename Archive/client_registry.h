#ifndef CLIENT_REGISTRY_H
#define CLIENT_REGISTRY_H
#pragma once
#include "lspmessage.pb-c.h"
#include "lsp_server.h"
#include <sys/socket.h>
#include <stdlib.h>

typedef struct client_registry_node{
	client_state_machine* csm;
	struct client_registry_node* next;
}client_registry_node;

/*
	if found, the csm pointer will be populated with the state machine and 0 will be returned. 
	Else, a -1 will be returned
*/
int find_by_connid(client_registry_node* reg, const uint32_t connid, client_state_machine* csm);
	
//if found, the csm pointer will be populated with the state machine and 0 will be returned. Else, a -1 will be returned
int find_by_clientaddr(client_registry_node* reg, const struct sockaddr_in clientaddr, client_state_machine* csm);

//removes the csm from the registry, and returns the pointer to the new registry head.
client_registry_node* remove_by_connid(client_registry_node* reg, uint32_t connid);
	

#endif
