#include "client_state_machine.h"

uint32_t connectionId = 1;

void initialize_csm(client_state_machine* csm,  const struct sockaddr_in clientaddr, const lsp_server server){
    csm->current_state = wait_to_send;
    csm->clientaddr = clientaddr;
    csm->missed_epochs = 0;
    csm->latest_epoch_seq = 0;
    csm->connid = get_next_connectionId();
    csm->latest_ACK_sent = createACK(csm->connid, 0);
    send_packet(csm->latest_ACK_sent, &(csm->clientaddr), server.socketfd);
    csm->latest_message_sent = NULL;
}

void free_csm(client_state_machine* csm){
	lspmessage_free_unpacked(csm->latest_ACK_sent, NULL);
	lspmessage_free_unpacked(csm->latest_message_sent, NULL);
	
}

LSPMessage* createACK(const int connid, const int seqnum){
	
	LSPMessage msg = LSPMESSAGE__INIT;
	LSPMessage *returnmsg = malloc(sizeof(msg));
	msg.connid = connid;
	msg.seqnum = 0;
	msg.payload.len = 0;
	memcpy(returnmsg, &msg, sizeof(msg));
	return returnmsg;
}


void wts_to_wtr(client_state_machine* csm, const lsp_server server){
  csm->current_state = wait_to_receive;
}  

void wtr_to_wts(client_state_machine* csm, const lsp_server server){
	LSPMessage* msg;
	
	switch(csm->current_state){
	case wait_to_receive:
		csm->current_state = wait_to_send;
		if(consume_next(msg, csm->outbox_queue) < 0){ //try to get the next message in the queue
			return; //there are no new messages, we're done
		}
		else { //we have a backlog of messages. Send latest
			send_msg(msg, csm, server);
		}
		break;
	case wait_to_send:
		break; //we've been called erroneously
	default:
		break;
	}
}

void send_msg(LSPMessage* message, client_state_machine* csm, const lsp_server server) {
	switch(csm->current_state){
	case wait_to_send:
		send_packet(message, &(csm->clientaddr), server.socketfd);
		if(message->payload.len != 0) { //Only change state if not sending an ACK
			lspmessage__free_unpacked(csm->latest_message_sent, NULL);
			csm->latest_message_sent = message;
			
			wts_to_wtr(csm, server);
		} else { //we just sent an ack
			lspmessage__free_unpacked(csm->latest_ACK_sent, NULL);
			csm->latest_ACK_sent = message;
		}
		break;
	case wait_to_receive: // If in wait_to_receive, we shouldn't send the message, we can add it to our backlog
		push_back(message, csm->outbox_queue);
		break;
	default:
		break;
	}
}
  

  
void receive_msg(LSPMessage* message, client_state_machine* csm, lsp_server* server) { 
	if(message->payload.len != 0){ //unsolicited data message
		if(message->seqnum > csm->latest_ACK_sent->seqnum) // only add the messaged to the inbox if its unique.
			push_back(message, server->inbox_queue);
		send_msg(createACK(message->connid, message->seqnum), csm, *server); //ACK it
	} else { //we've recieved an acknowledgement
		if(csm->current_state = wait_to_receive){ 
			if(message->seqnum >= (csm->latest_message_sent->seqnum)) { //if we recieve a seqnum equal later than the ack were expecting, then we can assume delivery
				wtr_to_wts(csm, *server);
				return;
			}
		}else{ //were wait_to_send, and  recieved an unessecary acknowledgement, discard 
			lspmessage__free_unpacked(message, NULL); 
			return;
		}
	}
}
