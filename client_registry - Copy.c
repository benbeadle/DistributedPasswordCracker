#include "client_registry.h"

/*
	if found, the csm pointer will be populated with the state machine and 0 will be returned. 
	Else, a -1 will be returned
*/
int find_by_connid(client_registry_node* reg, const uint32_t connid, client_state_machine* csm){
	while(reg != NULL) {
		if(reg->csm->connid == connid){
			csm = reg->csm
			return 0;
		}
		reg = reg->next;
	}
	return -1;
} 

//if found, the csm pointer will be populated with the state machine and 0 will be returned. Else, a -1 will be returned
int find_by_clientaddr(client_registry_node* reg, const sockaddr clientaddr, client_state_machine* csm){
	while(reg != NULL) {
		if(reg->csm->clientaddr == clientaddr){
			csm = reg->csm
			return 0;
		}
		reg = reg->next;
	}
	return -1;
} 

client_registry_node* remove_by_connid(client_registry_node* reg, uint32_t connid){
	if(reg == NULL){
		return;
	} else if(reg->csm->connid == connid){
		client_registry_node* next_node = reg->next;
		free_csm(reg->csm);
		free(reg)
		return next_node;
	} else {
		reg->next = remove_by_connid(reg->next, connid);
		return reg;
	}
}