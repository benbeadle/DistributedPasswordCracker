#ifndef CLIENT_STATE_MACHINE_H
#define CLIENT_STATE_MACHINE_H

#include "lsp_client.h"
#include "lsp_server.h"


typedef enum{
    wait_to_send,
    wait_to_receive
}State;

typedef struct{
    LSPMssage nextACK; //The latest message sent, keep sending the ack
    State current_state;
    LSPMssage latest_message_sent;
    int missed_epochs;
    int latest_epoch_seq;
    int connid;
    struct sockaddr_in clientaddr;
    linked_packet* outbox_head;
}client_state_machine;


client_state_machine* start_csm(sockaddr_in address. const Server* server);
void send_msg(LSPMssage message, client_state_machine csm);
void wts_to_wtr(client_state_machine csm);
void wtr_to_wts(client_state_machine csm);
<<<<<<< HEAD
void receive_msg(msg message, client_state_machine csm);

#endif
=======
void receive_msg(LSPMssage message, client_state_machine csm);
LSPMessage get_appropriate_ACK(LSPMssage message);
LSPMessage createACK(int connid);
>>>>>>> working on csm
