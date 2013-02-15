#include "client_registry.h"

/*
	if found, the lspu pointer will be populated with the state machine and 0 will be returned. 
	Else, a -1 will be returned
*/
int find_by_port(lsp_user_node* reg, const int port, lsp_user* lspu){
	while(reg != NULL) {
		if(reg->lspu->port == port){
			lspu = reg->lspu
			return 0;
		}
		reg = reg->next;
	}
	return -1;
} 


lsp_user* remove_by_port(lsp_user_node* reg, const int port){
	if(reg == NULL){
		return;
	} else if(reg->lspu->port == port){
		lsp_user_node* next_node = reg->next;
		free_lspu(reg->lspu);
		free(reg)
		return next_node;
	} else {
		reg->next = remove_by_port(reg->next, port);
		return reg;
	}
}

void apply_to_all ( lsp_user_node* reg, void (*f)(lsp_user_node*) ){
	lsp_user_node* reg_cpy = reg;
	while(reg_cpy != NULL){
		(*f)(reg_cpy->lspu);
		reg_cpy = reg_cpy->next;
	}
	return;
}