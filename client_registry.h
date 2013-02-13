#ifndef CLIENT_REGISTRY_H
#define CLIENT_REGISTRY_H

typedef struct{
	client_state_machine* csm;
	client_registry_node* next;
}client_registry_node;

/*
	if found, the csm pointer will be populated with the state machine and 0 will be returned. 
	Else, a -1 will be returned
*/
int find_by_connid(client_registry_node* reg, const uint32_t connid, client_state_machine* csm);
	
//if found, the csm pointer will be populated with the state machine and 0 will be returned. Else, a -1 will be returned
int find_by_clientaddr(client_registry_node* reg, const sockaddr clientaddr, client_state_machine* csm);

client_registry_node* remove_by_connid(client_registry_node* reg, uint32_t connid);
	

#endif
