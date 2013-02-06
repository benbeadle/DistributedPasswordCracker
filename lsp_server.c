#include lsp.h
#define BUFFER_LENGTH 100
#define FALSE 0
#define OUTBOX "/tmp/outbox"
#define INBOX "/tmp/inbox"

uint32_t connectionId = 1;

typedef struct{
	int socketfd;
	int port;
	int outboxfd;
	int inboxfd;
}lsp_server;

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
}client_state_machine;

lsp_server start_lsp_server(int port){
	lsp_server server;
	memset(&server, 0, sizeof(lsp_server));
	server.port = port;
	
	server.serveraddr.serveraddr_family      = AF_INET;
	server.serveraddr.sin_port				  = htons(port);
	server.serveraddr.serveraddr_addr.s_addr = htonl(INADDR_ANY);
	
	/* Allocate socket */
	server.socketfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (server.socketfd < 0) perror("can’t create socket");
	
	// set SO_REUSEADDR so that we can re-use not fully deallocated chatrooms
	int optval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	
	/* Bind the socket */
	if (bind(server.socketfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
		perror("can’t bind");
	
	/*create pipes*/
	
	if(mkfifo(INBOX,0777) < 0){ perror("cant create inbox pipes")};
	if(mkfifo(OUTBOX,0777) < 0){ perror("cant create outbox pipes")};

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
		server.outboxfd = open(OUTBOX, O_WRONLY);
		server.inboxfd = open(INBOX, O_RDONLY);
		return server;

	} else{
		/*
		We are the child
		*/
		/*
		Set up a signal handler sa1 for SIGTERM which will tell the fork to exit
		and a handler sa2 for SIGCHLD which will clean up children
		*/
		struct sigaction sa1;
		struct sigaction sa2;

		int inboxfd = open(INBOX, O_WRONLY);
		int outboxfd = open(OUTBOX, O_RDWR);
		
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
			
		struct sockaddr_in clientaddr; 	 /* Internet client address */
		int clientaddrlen = sizeof(clientaddr);
		char buffer[BUFFER_LENGTH];
	
		fd_set readfds, afds;
		int nfds = getdtablesize();
		FD_ZERO(&afds);  
		FD_SET(server.socketfd, &afds);
		FD_SET(outboxfd, &afds);
	

		for(;;){
			/*
				create a copy of the actual file descriptor set that we will listen for read or writes. 
				we do this becuase we do not wish &afds to be destroyed during the select statement, which will wait for 
				one of the file descriptors to be ready for a read or write operation
			*/
			memcpy(&readfds, &afds, sizeof(readfds));
			fprintf(stderr, "Waiting for select \n");
			select(nfds, &readfds, 0, 0, 0);
			fprintf(stderr, "Select unblocked \n");
			memset(buffer, 0, BUFFER_LENGTH);
			/*
				if the socket ready for reading, we will need to handle 
				this server command. Otherwise we probably just timed out
			*/
			if(FD_ISSET(server.socketfd, &readfds)){
				if( recv(server.socketfd, &buffer, BUFFER_LENGTH, 0, server.serveraddr, &clientaddr, &clientaddrlen) < 0){
					perror("recv()");
				}
			
				lsp_packet packet* = malloc(sizeof(lsp_packet));
				//TODO build packet from buffer (probs use protobufs to de-serialize)
				//TODO If connection request (connid == 0), create new csm
				//TODO Else find the csm with that connid and call correct method
				//TODO put message into server.inbox[0]
			
			}
			if(FD_ISSET(outboxfd, &readfds)){
				//TODO read from outbox into buffer
				//TODO build packet from buffer
				//TODO send packet with  if(sendto(server.socketfd, ....
			}
		}
	}
}

int get_next_connectionId() {
	return connectionId++;
}

client_state_machine start_csm(sockaddr_in address){
	client_state_machine csm = *malloc(sizeof(client_state_machine));
	csm.current_state = wait_to_receive;
	csm.clientaddr = address;
	csm.missed_epochs = 0;
	csm.lastest_epoch_seq = 0;
	csm.connid = get_next_connectionId();
	//TODO add ACK to outbox
	//TODO assign new ACK made to nextACk
	//WARNING Must check if lastest message is NULL
	csm.lastest_message_sent = NULL;
	
}

void send_msg(msg message, client_state_machine csm) {
    if(csm.state == State.wait_to_send) {
      //Only change state if not sending an ACK
      if(message.data != nil) { //TODO Find out what nil will look like in unmarshalled packet
        csm.latest_message_sent = message
        csm.state = State.wait_to_receive;
      }
	  //TODO Send the message
    }
    // If in wait_to_receive, we shouldn't send the message
  }
  
void receive_msg(msg message, client_state_machine csm) {
    if(csm.state == State.wait_to_send) {
      receiveData(message);
    }
    if(state == State.wait_to_receive) {
      if(message.data != nil) {
        receiveData(message);
      } else {
        if(message.seqnum == csm.latest_message_sent.seqnum) {
          csm.state = State.wait_to_send;
        }
      }
    }
}

void receive_data(msg message, client_state_machine csm) {
	//TODO Add message to inbox
	csm.nextACK = get_appropriate_ACK(message);
	//TODO Add nextACK to outbox;
	return;
}

msg get_appropriate_ACK(msg message){
	//TODO Based on the message, return the appropriate ack for the message
}

void epoch_tick() {
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
