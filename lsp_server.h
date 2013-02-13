#ifndef LSP_SERVER_H
#define LSP_SERVER_H

#include lsp.h
#include "lspmessage.pb-c.h"

typedef struct{
    int socketfd;
    int port;
    int outboxfd[2]; //from application to LSP
    int inboxfd[2]; //from LSP to application
	int cmdpipefd[2];
	queue_node* inbox_queue;
}lsp_server;

lsp_server start_lsp_server(int port);
LSPMessage* read_from_pipe(const int pipefd);
int send_through_pipe(LSPMessage* msg, const int pipefd);
int send_packet(LSPMessage* msg, const sockaddr* clientaddr, const int socket);
LSPMessage* recieve_packet(const int socket, struct sockaddr* clientaddr);
int get_next_connectionId();
void epoch_tick(client_state_machine* csm);

/*
int  lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id);
bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, uint32_t conn_id);
bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id);
*/
#endif
