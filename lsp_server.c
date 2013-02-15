#include "lsp_server.h"
srand(12345);
   
//TODO make sure packets we send have the right seqnum          

lsp_server start_lsp_server(int port){
    //allocate server struct
    lsp_server server;
    memset(&server, 0, sizeof(lsp_server));
	serv_ptr = &server;
	server.port = port;
	
	//Setup server address
	sockaddr_in serveraddr;
    serveraddr.serveraddr_family      = AF_INET;
    serveraddr.sin_port                = htons(port);
    serveraddr.serveraddr_addr.s_addr = htonl(INADDR_ANY);
    
    /* Allocate socket */
    server.socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server.socketfd < 0) perror("can�t create socket");
    
    // set SO_REUSEADDR so that we can re-use not fully deallocated chatrooms
    int optval = 1;
    setsockopt(server.socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    
    /* Bind the socket */
    if (bind(server.socketfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        perror("can�t bind");
    
    /*create pipes*/
    if(pipe(server.outboxfd) < 0) perror("cant create outbox pipe");
    if(pipe(server.inboxfd) <0) perror("cant create outbox pipe");
    if(pipe(server.cmdpipefd) <0) perror("cant create cmd pipe");

    server.serverpid = fork();
    if(server.serverpid == -1){
        perror("failed to fork");
    } else if (server.serverpid != 0){
        /*
            We are the parent, clean up and return
        */
        //close the socket, we wont use it
        close(server.socketfd);
        //close write end of inbox and read end of outbox
        close(server.inboxfd[0]);
        close(server.outboxfd[1]);
		//close the read end of the cmd pipe
		close(sever.cmdpipefd[1];
        
        return server;

    } else{
        /*
        We are the child
        */
		//close read end of inbox and write end of outbox
        close(server.inboxfd[1);
        close(server.outboxfd[0]);
		//close write end of cmd pipe
		close(server.cmdpipfd[0]);
		//initialize some things we need	
        struct sockaddr_in clientaddr;   /* Internet client address */
        int clientaddrlen = sizeof(clientaddr);
        char buffer[BUFFER_LENGTH];
		
		/*
        Set up a signal handler sa1 for SIGTERM which will tell the fork to exit
        and a handler sa2 for SIGCHLD which will clean up children
        */
        struct sigaction sa1;
        memset(&sa1, 0, sizeof(sa1));
        sa1.sa_handler = sigterm_hdl;
        if(sigaction(SIGTERM, &sa1, NULL)){
            perror("sigaction");
            return 1;
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
        FD_SET(server.socketfd, &rcopyfds);
        FD_SET(server.outbox[1], &rcopyfds);
		FD_SET(server.cmdpipefd[0], &rcopyfds);
		FD_SET(server.inbox[0], &wcopyfds);

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
            if(FD_ISSET(server.socketfd, &readfds)) {
                msg = recieve_packet(server.sockfd, &clientaddr);
				if(((double)rand()/(double)RAND_MAX) < packet_drop_rate ){ //see if we should just drop the packet for paramaterized drop rate
					lspmessage__free_unpacked( msg, NULL);
				}
				else if(msg->connid == 0){ //create new client for the connection
					node = malloc(sizeof(client_registry_node));
					node->csm = malloc(sizeof(client_state_machine));
					initialize_csm(node->csm, clientaddr, server);
					node->next = client_registry;
					client_registry = node;
					recieve_msg(msg, cliet_registry->csm, &server);
				}
                else{ //Else find the csm with that address and call recieve_msg(csm)		
					if(find_by_clientaddr(client_registry, clientaddr, csm) < 0){
						printf("WARNING: unable to find client for connection, dumping recieved packet");
						lspmessage__free_unpacked( msg, NULL);
					} else recieve_msg(msg, csm, &server);
				}
            
            }
            if(FD_ISSET(server.inbox[0], &writefds)){ //The main program wants the next message which is in the server's single inbox_list linked list. Write a method get_next_message(inbox_list);
				consume_next(msg, server.inbox_queue);
				send_through_pipe(msg, server.inbox[0])
				lspmessage__free_unpacked(msg, NULL);
			}
            if(FD_ISSET(server.outbox[1], &readfds)){ //The main program want to send out a message
                msg = read_from_pipe(server.outbox[1]);
				if(find_by_connid(client_registry, msg->connid, csm) < 0){ //try and get the csm with the messages connid
						printf("WARNING: unable to find client for packet, dumping recieved packet");
						lspmessage__free_unpacked( msg, NULL);
				} else send_msg(msg, csm, server);
            }
			if(FD_ISSET(server.cmdpipe[1], &readfds)){ //The main program wants to send a command
				/*
					To make my life easier, we are sending these commands as LSPMessages. connid represents the type of command,
					seqnum the parameter. Doubles will be sent as an int, and divided by 100.
					
				*/
				int i;
				double d;
				msg = read_from_pipe(server.cmdpipe[1]);
				switch(msg->connid){
				case SET_EPOCH_CNT:
					change_epoch_limit(msg->seqnum);
					break;
				case SET_EPOCH_LNTH:
					i = msg->seqnum;
					d = (double) i / (double) 100;
					changeperiodic(d);
					break;
				case SET_DROP_RATE:
					i = msg->seqnum;
					d = (double) i / (double) 100;
					if(change_drop_rate(&packet_drop_rate, d) < 0){
						perror("couldnt change packet drop rate");
					}
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
	
	return *msg;
}

int send_through_pipe(LSPMessage* msg, const int pipefd){
	int len = lspmessage__get_packed_size(msg);
	uint8_t* buf = malloc(len);
	lspmessage__pack(msg, buf);
	if(write(pipefd, buf, len) < 0) {
		perror("cant send packet");
		free(buf);
		return -1;
	}
	free(buf);
	return 0;
}

int send_packet(LSPMessage* msg, const sockaddr* clientaddr, const int socket){
	int len = lspmessage__get_packed_size(msg);
	uint8_t* buf = malloc(len);
	lspmessage__pack(msg, buf);
	if(sendto(socket, buf, len, 0,  clientaddr, sizeof(*(clientaddr))) < 0) {
		perror("cant send packet");
		free(buf);
		return -1;
	}
	free(buf);
	return 0;
}

LSPMessage* recieve_packet(const int socket, struct sockaddr* clientaddr){	
	LSPMessage* msg;

	// Read packed message from standard-input.
	uint8_t buf[BUFFER_LENGTH];
	size_t msg_len = recvfrom(socket, buf, BUFFER_LENGTH, 0,  clientaddr, sizeof(sizeof(*(clientaddr)))) ;
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
	
	return *msg;
}

int get_next_connectionId(){
    return connectionId++;
}

void epoch_tick(client_state_machine* csm){
    //TODO Wrap all code in a for each statement for each csm
	int largest_seqnum;
	if(csm->latest_message_sent.seqnum < csm->latest_ack_sent.seqnum){
		largest_seqnum = csm->latest_ack_sent.seqnum;
	}else{
		largest_seqnum = csm->latest_message_sent.seqnum;
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
    if(csm->state == wait_to_send) {
      csm->send(csm->nextACK); //The last data packet we sent was ACKed, just send the latest ack
    } else {
      csm->send(csm->nextACK);
      csm->send(csm->latest_message_sent); //Were waiting for an ack, resend the data and resend the ACK
    }
}

void sigepoch_hdl(int sig){
	client_registry_node* registry_cpy = client_registry;
	while(registry_cpy != NULL){
		epoch_tick(redistry_cpy->csm);
		registry_cpy = registry_cpy->next;
	}
};

void sigterm_hdl(int sig){
	close(serv_ptr->outboxfd[1]);
	close(serv_ptr->inboxfd[0]);
	close(serv_ptr->cmdpipefd[1]);
	exit(0);
};

void change_epoch_limit(int times){
	epoch_cnt = times;
}	

void change_drop_rate(double new_rate){
	drop_rate = new_rate;
}