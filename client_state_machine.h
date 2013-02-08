typedef enum{
    wait_to_send,
    wait_to_receive
}State;

typedef struct{
    msg nextACK; //The latest message sent, keep sending the ack
    State current_state;
    msg latest_message_sent;
    int missed_epochs;
    int latest_epoch_seq;
    int connid;
    struct sockaddr_in clientaddr;
    linked_packet* outbox_head;
}client_state_machine;


client_state_machine start_csm(sockaddr_in address);
void send_msg(msg message, client_state_machine csm);
void wts_to_wtr(client_state_machine csm);
void wtr_to_wts(client_state_machine csm);
void receive_msg(msg message, client_state_machine csm);