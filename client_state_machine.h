#ifndef CLIENT_STATE_MACHINE_H
#define CLIENT_STATE_MACHINE_H

#include "lsp_client.h"
#include "lsp_server.h"


typedef enum{
    wait_to_send,
    wait_to_receive
}State;

typedef struct{
    LSPMssage* latest_ACK_sent; //The latest message sent, keep sending the ack
    State current_state;
    LSPMssage* latest_message_sent;
    int missed_epochs;
    int latest_epoch_seq;
    int connid;
	int last_seqnum_used;
    struct sockaddr clientaddr;
    queue_node* outbox_queue;
}client_state_machine;


void initialize_csm(client_state_machine* csm,  const sockaddr clientaddr, const Server server);
void free_csm(client_state_machine* csm);  
LSPMessage* createACK(const int connid, const int seqnum);
void send_msg(LSPMssage* message, client_state_machine* csm, const Server server);
void wts_to_wtr(client_state_machine* csm, const Server server);
void wtr_to_wts(client_state_machine* csm, const Server server);
void receive_msg(LSPMssage* message, client_state_machine* csm, Server* server);


/*
//Old stuff
client_state_machine* start_csm(sockaddr_in address. const Server* server);
void send_msg(LSPMssage message, client_state_machine csm);
void wts_to_wtr(client_state_machine csm);
void wtr_to_wts(client_state_machine csm);
void receive_msg(LSPMssage message, client_state_machine csm);
LSPMessage get_appropriate_ACK(LSPMssage message);
LSPMessage createACK(int connid);
void free_csm(client_state_machine* csm);
*/
#endif
