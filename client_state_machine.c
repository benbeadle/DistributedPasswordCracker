#include "client_state_machine.h"

uint32_t connectionId = 1;

void initialize_csm(client_state_machine* csm,  const sockaddr clientaddr, const Server server){
    csm->current_state = State.wait_to_send;
    csm->clientaddr = clientaddr;
    csm->missed_epochs = 0;
    csm->lastest_epoch_seq = 0;
    csm->connid = get_next_connectionId();
    csm->nextACK = createACK(csm->connid, 0);
    send_packet(csm->nextACK, &(csm->clientaddr), server.socketfd);
    csm.lastest_message_sent = NULL;
}

void free_csm(client_state_machine* csm){
	lspmessage_free_unpacked(latest_ACK_sent, NULL);
	lspmessage_free_unpacked(latest_message_sent, NULL);
	
}

LSPMessage* createACK(const int connid, const int seqnum){
	LSPMessage msg = LSPMESSAGE__INIT;
	msg.connid = connid;
	msg.seqnum = 0;
	msg.payload.len = 0;
	return &msg;
}

void send_msg(LSPMssage* message, client_state_machine* csm, const Server server) {
	switch(csm->state){
	case wait_to_send:
		send_packet(message, &(csm->clientaddr), server.socketfd);
		if(message->data->len != 0) { //Only change state if not sending an ACK
			lspmessage__free_unpacked(csm->latest_message_sent, NULL);
			csm->latest_message_sent = message
			
			wts_to_wtr(csm, server);
		} else { //we just sent an ack
			lspmessage__free_unpacked(csm->latest_ACK_sent, NULL);
			csm->latest_ACK_sent = message;
		}
		break;
	case wait_to_recieve: // If in wait_to_receive, we shouldn't send the message, we can add it to our backlog
		push_back(message, csm->outbox_queue);
		break;
	case default:
		break;
	}
}
  
void wts_to_wtr(client_state_machine* csm, const Server server){
  csm->state = State.wait_to_recieve;
}  

void wtr_to_wts(client_state_machine* csm, const Server server){
	LSPMessage* msg;
	
	switch(csm->state){
	case wait_to_recieve:
		csm->state = State.wait_to_send;
		if(consume_next(msg, csm->outbox_queue) < 0){ //try to get the next message in the queue
			return; //there are no new messages, we're done
		}
		else { //we have a backlog of messages. Send latest
			send_msg(msg, csm, server);
		}
		break;
	case wait_to_send:
		break; //we've been called erroneously
	case default:
		break;
	}
}
  
void receive_msg(LSPMssage* message, client_state_machine* csm, Server* server) { 
	if(message->data->len != 0){ //unsolicited data message
		if(message->seqnum > csm->latest_ACK_sent->seqnum) // only add the messaged to the inbox if its unique.
			push_back(message, server->inbox_queue);
		send_msg(createACK(message->connid, message->seqnum), csm, *server); //ACK it
	} else { //we've recieved an acknowledgement
		if(csm->state = wait_to_recieve){ 
			if(message.seqnum >= csm->latest_message_sent->seqnum) { //if we recieve a seqnum equal later than the ack were expecting, then we can assume delivery
				wtr_to_wts(csm, *server);
				return;
			}
		}else{ //were wait_to_send, and  recieved an unessecary acknowledgement, discard 
			lspmessage__free_unpacked(message, NULL); 
			return;
		}
	}
}