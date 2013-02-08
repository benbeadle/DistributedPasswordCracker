#include "client_state_machine.h"

client_state_machine* start_csm(sockaddr address, const Server* server){
    client_state_machine* csm = malloc(sizeof(client_state_machine));
    csm->current_state = State.wait_to_receive;
    csm->clientaddr = address;
    csm->missed_epochs = 0;
    csm->lastest_epoch_seq = 0;
    csm->connid = get_next_connectionId();
    csm->nextACK = createACK(csm.connid);
    send_packet(csm->nextACK, &clientaddr, server->socketfd);
    csm.lastest_message_sent = NULL;
	return csm;
}

LSPMessage createACK(const int connid){
	LSPMessage msg = LSPMESSAGE__INIT;
	msg.connid = connid;
	msg.seqnum = 0;
	msg.payload.len = 0;
	return msg;
}

void send_msg(LSPMssage message, client_state_machine* csm, Server* server) {
    if(csm.state == State.wait_to_send) {
      //Only change state if not sending an ACK
      if(message.data.len == 0) { 
        csm.latest_message_sent = message
        wts_to_wtr(csm);
      }
	  send_packet(message, &(csm->clientaddr), server->socketfd);
    }
    else{ // If in wait_to_receive, we shouldn't send the message, we can add it to our backlog
      add_packet_end(message, csm.outbox_head); //TODO write add_to_csm_outbox
    }
  }
  
void wts_to_wtr(client_state_machine* csm){
  csm->state = State.wait_to_recieve;
}  

void wtr_to_wts(client_state_machine* csm){
  if(csm->state == State.wait_to_recieve){
    if(csm->outbox_head == NULL){
      csm->state = State.wait_to_receive;
    } else { //we have a backlog of messages. Send latest and return to wtr
      //TODO get next message from csm outbox... message msg = get_next_message(csm);
      //TODO send message
      csm->latest_message_sent = msg
      csm->state = State.wait_to_receive;
    }
  } else return //this function was called erroneously
  
  
}
  
void receive_msg(LSPMssage message, client_state_machine* csm, Server* server) {
    if(csm.state == State.wait_to_send) {
      if(message.data != nil){
        add_packet_end(message, server->outbox_queue);
        csm.nextACK = get_appropriate_ACK(message);
      }
      else{
        //we've recieved an unessecary acknowledgement, discard
        return;
      }
    }
    if(state == State.wait_to_receive) {
      if(message.data != nil) {
        add_to_inbox(message);
        csm.nextACK = get_appropriate_ACK(message);
      } else {
        if(message.seqnum >= csm.latest_message_sent.seqnum) { //if we recieve a seqnum later than the ack were expecting, then we can assume delivery
           //TODO send nextACK
          wtr_to_wts(csm);
        }
      }
    }
}

LSPMssage get_appropriate_ACK(const LSPMssage message){
    LSPMessage 
}
