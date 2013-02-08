#ifndef LSP_SERVER_H
#define LSP_SERVER_H

#include lsp.h
#include "lspmessage.pb-c.h"

typedef struct{
    int socketfd;
    int port;
    int outboxfd[2];
    int inboxfd[2];
}lsp_server;

lsp_server start_lsp_server(int port);
size_t read_buffer (unsigned max_length, uint8_t *out);
LSPMessage read_from_pipe(const int pipefd);
LSPMessage send_through_pipe(const int pipefd);
int recieve_packet(LSPMessage msg, const int pipefd);
int send_packet(LSPMessage msg, const sockaddr* clientaddr, const int socket);
int get_next_connectionId();
void epoch_tick();
msg get_appropriate_ACK(msg message);

#endif
