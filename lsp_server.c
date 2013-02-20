#include "lsp_server.h"
#include <stdlib.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <time.h>


client_registry_node * client_registry;
lsp_server * server;
timer_t timerid;
int max_missed_epoch_limit = _EPOCH_CNT;
uint32_t connectionId = 1;

//TODO make sure packets we send have the right seqnum          

lsp_server* start_lsp_server(int port){
	srand(12345);
    //allocate server struct
    memset(server, 0, sizeof(lsp_server));
	server->port = port;
	
	//Setup server address
	sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    /* Allocate socket */
    server->socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server->socketfd < 0) perror("can’t create socket");
    
    // set SO_REUSEADDR so that we can re-use not fully deallocated chatrooms
    int optval = 1;
    setsockopt(server->socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    
    /* Bind the socket */
    if (bind(server->socketfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        perror("can’t bind");
    
    /*create pipes*/
    if(pipe(server->outboxfd) < 0) perror("cant create outbox pipe");
    if(pipe(server->inboxfd) <0) perror("cant create outbox pipe");
    if(pipe(server->cmdpipefd) <0) perror("cant create cmd pipe");

    int serverpid = fork();
    if(serverpid == -1){
        perror("failed to fork");
    } else if (serverpid != 0){
        /*
            We are the parent, clean up and return
        */
        //close the socket, we wont use it
        close(server->socketfd);
        //close write end of inbox and read end of outbox
        close(server->inboxfd[0]);
        close(server->outboxfd[1]);
		//close the read end of the cmd pipe
		close(server->cmdpipefd[1]);
        
        return server;

    } else{
        /*
        We are the child
        */
		//close read end of inbox and write end of outbox
        close(server->inboxfd[1]);
        close(server->outboxfd[0]);
		//close write end of cmd pipe
		close(server->cmdpipefd[0]);
		//initialize some things we need	
        struct sockaddr_in clientaddr;   /* Internet client address */
        socklen_t clientaddrlen = (socklen_t) sizeof(clientaddr);
        char buffer[BUFFER_LENGTH];
		double drop_rate = _DROP_RATE;
		
		/*
        Set up a signal handler sa1 for SIGTERM which will tell the fork to exit
        and a handler sa2 for SIGCHLD which will clean up children
        */
        struct sigaction sa1;
        memset(&sa1, 0, sizeof(sa1));
        sa1.sa_handler = sigterm_hdl;
        if(sigaction(SIGTERM, &sa1, NULL)){
            perror("sigaction");
        }
		
		/*
			Set Epoch Handler and start epoch timers
		*/
		if(setinterrupt() == -1){
			perror("Failed to setup SIGALRM handler");
		}
		if(setperiodic(_EPOCH_LTH) == -1){
			perror("Failed to setup periodic interrupt");
		}
		
		/*
			Get ready to enter select statement
		*/
        fd_set readfds, writefds, rcopyfds, wcopyfds;
        int nfds = getdtablesize();
        FD_ZERO(&readfds);  
        FD_ZERO(&writefds);  
        FD_ZERO(&rcopyfds);  
        FD_ZERO(&wcopyfds);  
        FD_SET(server->socketfd, &rcopyfds);
        FD_SET(server->outboxfd[1], &rcopyfds);
		FD_SET(server->cmdpipefd[0], &rcopyfds);
		FD_SET(server->inboxfd[0], &wcopyfds);

        for(;;){
			/*
				some temp variables that may be useful
			*/
			LSPMessage* msg;
			client_state_machine* csm;
			client_registry_node* node;
            /*
                create a copy of the actual file descriptor set that we will listen for read or writes. 
                we do this becuase we do not wish &afds to be destroyed during the select statement, which will wait for 
                one of the file descriptors to be ready for a read or write operation
            */
            memcpy(&readfds, &rcopyfds, sizeof(rcopyfds));
            memcpy(&writefds, &wcopyfds, sizeof(wcopyfds));
            fprintf(stderr, "Waiting for select \n");
            select(nfds, &readfds, &writefds, 0, 0);
            fprintf(stderr, "Select unblocked \n");
            memset(buffer, 0, BUFFER_LENGTH);
            /*
                if the socket ready for reading, we will need to handle 
                this server command. Otherwise we probably just timed out
            */
            if(FD_ISSET(server->socketfd, &readfds)) {
                msg = recieve_packet(server->socketfd, (sockaddr*) &clientaddr, clientaddrlen);
				if(((double)rand()/(double)RAND_MAX) < drop_rate ){ //see if we should just drop the packet for paramaterized drop rate
					lspmessage__free_unpacked( msg, NULL);
				}
				else if(msg->connid == 0){ //create new client for the connection
					node = static_cast<client_registry_node*>(malloc(sizeof(client_registry_node)));
					node->csm = static_cast<client_state_machine*>(malloc(sizeof(client_state_machine)));
					initialize_csm(node->csm, clientaddr, server);
					node->next = client_registry;
					client_registry = node;
					receive_msg(msg, client_registry->csm, server);
				}
                else{ //Else find the csm with that address and call recieve_msg(csm)		
					if(find_by_clientaddr(client_registry, clientaddr, csm) < 0){
						printf("WARNING: unable to find client for connection, dumping recieved packet");
						lspmessage__free_unpacked( msg, NULL);
					} else receive_msg(msg, csm, server);
				}
            
            }
            if(FD_ISSET(server->inboxfd[0], &writefds)){ //The main program wants the next message which is in the server's single inbox_list linked list.
				consume_next(msg, server->inbox_queue);
				send_through_pipe(msg, server->inboxfd[0]);
				lspmessage__free_unpacked(msg, NULL);
			}
            if(FD_ISSET(server->outboxfd[1], &readfds)){ //The main program want to send out a message
                msg = read_from_pipe(server->outboxfd[1]);
				if(find_by_connid(client_registry, msg->connid, csm) < 0){ //try and get the csm with the messages connid
						printf("WARNING: unable to find client for packet, dumping recieved packet");
						lspmessage__free_unpacked( msg, NULL);
				} else send_msg(msg, csm, server);
            }
			if(FD_ISSET(server->cmdpipefd[1], &readfds)){ //The main program wants to send a command
				/*
					To make my life easier, we are sending these commands as LSPMessages. connid represents the type of command,
					seqnum the parameter. Doubles will be sent as an int, and divided by 100.
					
				*/
				int i;
				double d;
				msg = read_from_pipe(server->cmdpipefd[1]);
				switch(msg->connid){
				case SET_EPOCH_CNT:
					max_missed_epoch_limit = msg->seqnum;
					break;
				case SET_EPOCH_LNTH:
					i = msg->seqnum;
					d = (double) i / (double) 100;
					changeperiodic(d);
					break;
				case SET_DROP_RATE:
					i = msg->seqnum;
					d = (double) i / (double) 100;
					drop_rate = d;
					break;
				case DROP_CONN:
					client_registry = remove_by_connid(client_registry, msg->seqnum);
				default:
					printf("WARNING: Undefined command packet \n");
				}
				lspmessage__free_unpacked(msg, NULL);
			}
        }
    }
}

LSPMessage* read_from_pipe(const int pipefd){
	LSPMessage *msg;

	// Read packed message from standard-input.
	uint8_t buf[BUFFER_LENGTH];
	read(pipefd, buf, BUFFER_LENGTH);
	size_t msg_len = sizeof(buf);

	// Unpack the message using protobuf-c.
	msg = lspmessage__unpack(NULL, msg_len, buf);   
	if (msg == NULL){
	  fprintf(stderr, "error unpacking incoming message\n");
	  exit(1);
	}
	
	return msg;
}

int send_through_pipe(LSPMessage* msg, const int pipefd){
	int len = lspmessage__get_packed_size(msg);
	uint8_t* buf = static_cast<uint8_t*> (malloc(len));
	lspmessage__pack(msg, buf);
	if(write(pipefd, buf, len) < 0) {
		perror("cant send packet");
		free(buf);
		return -1;
	}
	free(buf);
	return 0;
}

int send_packet(LSPMessage* msg, const struct sockaddr_in* clientaddr, const int socket, socklen_t clientaddrlen){
	int len = lspmessage__get_packed_size(msg);
	uint8_t* buf = static_cast<uint8_t*> (malloc(len));
	lspmessage__pack(msg, buf);
	if(sendto(socket, buf, len, 0,  (sockaddr*) clientaddr, clientaddrlen) < 0) {
		perror("cant send packet");
		free(buf);
		return -1;
	}
	free(buf);
	return 0;
}

LSPMessage* recieve_packet(const int socket, struct sockaddr* clientaddr, socklen_t clientaddrlen){	
	LSPMessage* msg;

	// Read packed message from standard-input.
	uint8_t buf[BUFFER_LENGTH];
	size_t msg_len = recvfrom(socket, buf, BUFFER_LENGTH, 0,  clientaddr, &clientaddrlen) ;
	if( msg_len < 0){
		perror("cant recieve packet");
		return NULL;
	}

	// Unpack the message using protobuf-c.
	msg = lspmessage__unpack(NULL, msg_len, buf);   
	if (msg == NULL){
	  fprintf(stderr, "error unpacking incoming message\n");
	  exit(1);
	}
	
	return msg;
}

int get_next_connectionId(){
    return connectionId++;
}

void epoch_tick(client_state_machine* csm){
    //TODO Wrap all code in a for each statement for each csm
	int largest_seqnum;
	if( (csm->latest_message_sent->seqnum) < (csm->latest_ACK_sent->seqnum)){
		largest_seqnum = csm->latest_ACK_sent->seqnum;
	}else{
		largest_seqnum = csm->latest_message_sent->seqnum;
	}
	
    if(largest_seqnum == csm->latest_epoch_seq) {
      csm->missed_epochs++;
    } else {
      csm->latest_epoch_seq = largest_seqnum;
      csm->missed_epochs = 0;
    }
    if(csm->missed_epochs == max_missed_epoch_limit) {
      //terminate connection
      return;
    }
    if(csm->current_state == wait_to_send) { 
      send_msg(csm->latest_ACK_sent, csm, server); //The last data packet we sent was ACKed, just send the latest ack
    } else {
      send_msg(csm->latest_ACK_sent, csm, server);
      send_msg(csm->latest_message_sent, csm, server); //Were waiting for an ack, resend the data and resend the ACK
    }
}

void sigepoch_hdl(int sig, siginfo_t* info, void* parms){
	client_registry_node* registry_cpy = client_registry;
	while(registry_cpy != NULL){
		epoch_tick(registry_cpy->csm);
		registry_cpy = registry_cpy->next;
	}
};

void sigterm_hdl(int sig){
	close(server->outboxfd[1]);
	close(server->inboxfd[0]);
	close(server->cmdpipefd[1]);
	exit(0);
};


/************************
			CSM STUFF
**************************/

void initialize_csm(client_state_machine* csm,  const struct sockaddr_in clientaddr, lsp_server* server){
    csm->current_state = wait_to_send;
    csm->clientaddr = clientaddr;
    csm->missed_epochs = 0;
    csm->latest_epoch_seq = 0;
    csm->connid = get_next_connectionId();
    csm->latest_ACK_sent = createACK(csm->connid, 0);
    send_packet(csm->latest_ACK_sent, &(csm->clientaddr), server->socketfd, (socklen_t) sizeof(csm->clientaddr));
    csm->latest_message_sent = NULL;
	return;
}

void free_csm(client_state_machine* csm){
	lspmessage__free_unpacked(csm->latest_ACK_sent, NULL);
	lspmessage__free_unpacked(csm->latest_message_sent, NULL);
	
}

LSPMessage* createACK(const int connid, const int seqnum){
	
	LSPMessage msg = LSPMESSAGE__INIT;
	LSPMessage *returnmsg = static_cast<LSPMessage*>(malloc(sizeof(LSPMessage)));
	msg.connid = connid;
	msg.seqnum = 0;
	msg.payload.len = 0;
	memcpy(returnmsg, &msg, sizeof(msg));
	return returnmsg;
}


void wts_to_wtr(client_state_machine* csm, lsp_server* server){
  csm->current_state = wait_to_receive;
}  

void wtr_to_wts(client_state_machine* csm, lsp_server* server){
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

void send_msg(LSPMessage* message, client_state_machine* csm, lsp_server* server) {
	switch(csm->current_state){
	case wait_to_send:
		send_packet(message, &(csm->clientaddr), server->socketfd, (socklen_t) sizeof(csm->clientaddr) );
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
		send_msg(createACK(message->connid, message->seqnum), csm, server); //ACK it
	} else { //we've recieved an acknowledgement
		if(csm->current_state = wait_to_receive){ 
			if(message->seqnum >= (csm->latest_message_sent->seqnum)) { //if we recieve a seqnum equal later than the ack were expecting, then we can assume delivery
				wtr_to_wts(csm, server);
				return;
			}
		}else{ //were wait_to_send, and  recieved an unessecary acknowledgement, discard 
			lspmessage__free_unpacked(message, NULL); 
			return;
		}
	}
}

void free_lsp_user( lsp_user* lspu){
	free_queue(lspu->inbox_queue);
	close(lspu->outboxfd[0]);
	close(lspu->outboxfd[1]); 
	close(lspu->inboxfd[0]); 
	close(lspu->inboxfd[1]);
	close(lspu->cmdpipefd[0]);
	close(lspu->cmdpipefd[1]);
	free(lspu);
}


/****

EPOCH STUFF


****/

int setinterrupt(){
	struct sigaction act;
	
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = sigepoch_hdl;
	if((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGALRM, &act, NULL) == -1))
		return 0;
}

int setperiodic(double sec){
	//timer_t timerid;
	itimerspec value;
	
	if(timer_create(CLOCK_REALTIME, NULL, &timerid) == -1)
		return -1;
	
	value.it_interval.tv_sec = (long)sec;
	value.it_interval.tv_nsec = (sec - value.it_interval.tv_sec)*BILLION;
	if(value.it_interval.tv_nsec >= BILLION){
		value.it_interval.tv_sec++;
		value.it_interval.tv_nsec -= BILLION;
	}
	
	value.it_value = value.it_interval;
	return timer_settime(timerid, 0, &value, NULL);
}

int changeperiodic(double sec){
	struct itimerspec value;
	
	value.it_interval.tv_sec = (long)sec;
	value.it_interval.tv_nsec = (sec - value.it_interval.tv_sec)*BILLION;
	if(value.it_interval.tv_nsec >= BILLION){
		value.it_interval.tv_sec++;
		value.it_interval.tv_nsec -= BILLION;
	}
	
	value.it_value = value.it_interval;
	return timer_settime(timerid, 0, &value, NULL);
}

/************************
			lsp user directory stuff
**************************/

/*
	if found, the lspu pointer will be populated with the state machine and 0 will be returned. 
	Else, a -1 will be returned
*/
int find_by_port(lsp_user_node* reg, const int port, lsp_user* lspu){
	while(reg != NULL) {
		if(reg->lspu->port == port){
			lspu = reg->lspu;
			return 0;
		}
		reg = reg->next;
	}
	return -1;
} 

//returns the new head of the of the registry
lsp_user_node* remove_by_port(lsp_user_node* reg, const int port){
	if(reg == NULL){
		return NULL;
	} else if(reg->lspu->port == port){
		lsp_user_node* next_node = reg->next;
		free_lsp_user(reg->lspu);
		free(reg);
		return next_node;
	} else {
		reg->next = remove_by_port(reg->next, port);
		return reg;
	}
}

void apply_to_all ( lsp_user_node* reg, void (*f)(lsp_user*) ){
	lsp_user_node* reg_cpy = reg;
	while(reg_cpy != NULL){
		(*f)(reg_cpy->lspu);
		reg_cpy = reg_cpy->next;
	}
	return;
}

/************************
			client_registry stuff
**************************/

/*
	if found, the csm pointer will be populated with the state machine and 0 will be returned. 
	Else, a -1 will be returned
*/
int find_by_connid(client_registry_node* reg, const uint32_t connid, client_state_machine* csm){
	while(reg != NULL) {
		if(reg->csm->connid == connid){
			csm = reg->csm;
			return 0;
		}
		reg = reg->next;
	}
	return -1;
} 

//if found, the csm pointer will be populated with the state machine and 0 will be returned. Else, a -1 will be returned
int find_by_clientaddr(client_registry_node* reg, const struct sockaddr_in clientaddr, client_state_machine* csm){
	while(reg != NULL) {
		//char *ip1 = inet_ntoa(( (struct sockaddr_in) clientaddr).sin_addr);
		if( (reg->csm->clientaddr.sin_addr.s_addr) == clientaddr.sin_addr.s_addr){ //TODO resolve the casting problem you know you created
			csm = reg->csm;
			return 0;
		}
		reg = reg->next;
	}
	return -1;
} 

client_registry_node* remove_by_connid(client_registry_node* reg, uint32_t connid){
	if(reg == NULL){
		return NULL;
	} else if(reg->csm->connid == connid){
		client_registry_node* next_node = reg->next;
		free_csm(reg->csm);
		free(reg);
		return next_node;
	} else {
		reg->next = remove_by_connid(reg->next, connid);
		return reg;
	}
}
