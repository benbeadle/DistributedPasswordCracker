#ifndef CLIENT_STATE_MACHINE_H
#define CLIENT_STATE_MACHINE_H
#pragma once
#include <netinet/in.h>
#include <stdlib.h>
#include "lspmessage.pb-c.h"
#include "queue.h"


typedef enum{
    wait_to_send,
    wait_to_receive
}State;

typedef struct client_state_machine{
    LSPMessage* latest_ACK_sent; //The latest message sent, keep sending the ack
    State current_state;
    LSPMessage* latest_message_sent;
    int missed_epochs;
    int latest_epoch_seq;
    int connid;
	int last_seqnum_used;
    struct sockaddr_in clientaddr;
    queue_node* outbox_queue;
}client_state_machine;
#include "lsp_server.h"

void initialize_csm(client_state_machine* csm,  const struct sockaddr_in clientaddr, const lsp_server server);
void free_csm(client_state_machine* csm);
LSPMessage* createACK(const int connid, const int seqnum);
void send_msg(LSPMessage* message, client_state_machine* csm, const lsp_server server);
void wts_to_wtr(client_state_machine* csm, const lsp_server server);
void wtr_to_wts(client_state_machine* csm, const lsp_server server);
void receive_msg(LSPMessage* message, client_state_machine* csm, lsp_server* server);

#endif

