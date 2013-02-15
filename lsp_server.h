#ifndef LSP_SERVER_H
#define LSP_SERVER_H

#include lsp.h
#include "lspmessage.pb-c.h"

#define BUFFER_LENGTH 100
#define FALSE 0
#define SET_EPOCH_CNT 0
#define SET_EPOCH_LNTH 1
#define SET_DROP_RATE 2
#define DROP_CONN 3

lsp_server start_lsp_server(int port);
LSPMessage* read_from_pipe(const int pipefd);
int send_through_pipe(LSPMessage* msg, const int pipefd);
int send_packet(LSPMessage* msg, const sockaddr* clientaddr, const int socket);
LSPMessage* recieve_packet(const int socket, struct sockaddr* clientaddr);
int get_next_connectionId();
void epoch_tick(client_state_machine* csm);
void sigepoch_hdl(int sig);
void sigterm_hdl(int sig);
void change_epoch_limit(int times);
void change_drop_rate(double new_rate);

client_registry_node* client_registry;
lsp_server* serv_ptr;
timer_t timerid;


/*
int  lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id);
bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, uint32_t conn_id);
bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id);
*/
#endif
