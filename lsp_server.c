#include "lsp_server.h"

#define BUFFER_LENGTH 100
#define FALSE 0
#define OUTBOX "/tmp/outbox"
#define INBOX "/tmp/inbox"

//TODO do we need all the free(LSPMessage*) to be lspmessage__free_unpacked(LSPMessage*, NULL) ?
//TODO we need those sighandlers back in
//TODO Epoch stuff
//TDOD try and compile! (Ouch!)
//TODO fix headers

lsp_server start_lsp_server(int port){
    //allocate server struct
    lsp_server server;
    memset(&server, 0, sizeof(lsp_server));
    server.port = port;
    server.serveraddr.serveraddr_family      = AF_INET;
    server.serveraddr.sin_port                = htons(port);
    server.serveraddr.serveraddr_addr.s_addr = htonl(INADDR_ANY);
    
    /* Allocate socket */
    server.socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server.socketfd < 0) perror("can’t create socket");
    
    // set SO_REUSEADDR so that we can re-use not fully deallocated chatrooms
    int optval = 1;
    setsockopt(server.socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    
    /* Bind the socket */
    if (bind(server.socketfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        perror("can’t bind");
    
    /*create pipes*/
    if(pipe(server.outboxfd) < 0) perror("cant create outbox pipe");
    if(pipe(server.inboxfd) <0) perror("cant create outbox pipe");
    

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
        
        return server;

    } else{
        /*
        We are the child
        */
        /*
        Set up a signal handler sa1 for SIGTERM which will tell the fork to exit
        and a handler sa2 for SIGCHLD which will clean up children
        */
		/*
        struct sigaction sa1;
        struct sigaction sa2;
        linked_packet * inbox_list;
        
        memset(&sa1, 0, sizeof(sa1));
        sa1.sa_handler = sigterm_hdl;
        if(sigaction(SIGTERM, &sa1, NULL)){
            perror("sigaction");
            return 1;
        }
        memset(&sa2, 0, sizeof(sa2));
        sa2.sa_handler = sigchld_hdl;
        if(sigaction(SIGCHLD, &sa2, NULL)){
            perror("sigaction");
            return 1;
        }
		*/
        
        //close read end of inbox and write end of outbox
        close(server.inboxfd[1);
        close(server.outboxfd[0]);
            
		//initialize some things we need	
        struct sockaddr_in clientaddr;   /* Internet client address */
        int clientaddrlen = sizeof(clientaddr);
        char buffer[BUFFER_LENGTH];
		client_registry_node* client_registry;
		
		
    
        fd_set readfds, writefds, rcopyfds, wcopyfds;
        int nfds = getdtablesize();
        FD_ZERO(&readfds);  
        FD_ZERO(&writefds);  
        FD_ZERO(&rcopyfds);  
        FD_ZERO(&wcopyfds);  
        FD_SET(server.socketfd, &rcopyfds);
        FD_SET(server.inbox[0], &wcopyfds);
        FD_SET(server.outbox[1], &rcopyfds);

        for(;;){
			/*
				some temp variables that may be useful
			*/
			LSPMessage* msg;
			client_state_machine* csm;
			client_registry_node* node;
            /*
                create a copy of the actual file descriptor set that we will listen for read or writes. 
                we do this becuase we do not wish &afds to be destroyed during the select statement, which             will wait for 
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
				if(msg->connid == 0){ //create new client for the connection
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
				free(msg);
			}
            if(FD_ISSET(server.outbox[1], &readfds)){ //The main program want to send out a message
                msg = read_from_pipe(server.outbox[1]);
                //TODO call send_msg(csm) on the correct state machine
				if(find_by_connid(client_registry, msg->connid, csm) < 0){ //try and get the csm with the messages connid
						printf("WARNING: unable to find client for packet, dumping recieved packet");
						lspmessage__free_unpacked( msg, NULL);
				} else send_msg(msg, csm, server);
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
	if(recvfrom(socket, buf, BUFFER_LENGTH, 0,  clientaddr, sizeof(sizeof(*(clientaddr)))) < 0){
		perror("cant recieve packet");
		return NULL;
	}

	size_t msg_len = read_buffer(BUFFER_LENGTH, buf);

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

void epoch_tick(){
    //TODO Wrap all code in a for each statement for each csm
    if(latest_message_sent.seqnum == latest_epoch_seq) {
      missed_epochs++;
    } else {
      latest_epoch_seq = latest_message_sent.seqnum;
      missed_epochs = 0;
    }
    if(missed_epochs == MAX_MISSED_EPOCH_LIMIT) {
      //terminate connection
      return;
    }
    if(state == wait_to_send) {
      send(nextACK);
    } else {
      send(nextACK);
      send(latest_message_sent);
    }
}

