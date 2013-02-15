#ifndef CLIENT_REGISTRY_H
#define CLIENT_REGISTRY_H

typedef struct{
	lsp_user* lspu;
	lsp_user_node* next;
}lsp_user_node;

/*
	if found, the lspu pointer will be populated with the state machine and 0 will be returned. 
	Else, a -1 will be returned
*/
int find_by_port(lsp_user_node* reg, const int port, lsp_user* lspu);

//removes the specified lsp_user from the directory and returns the new pointer to the registry
lsp_user* remove_by_port(lsp_user_node* reg, const int port);
	
//applies a function to all members of the registry.
void apply_to_all ( lsp_user_node* reg, void (*f)(lsp_user_node*) );
#endif 