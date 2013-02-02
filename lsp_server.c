#include lsp.h
#define BUFFER_LENGTH 100
#define FALSE 0

typedef struct{
	int socketfd;
	struct sockaddr_in serveraddr;          /* Internet endpoint address */
	int inboxfds[2];
	int outboxfds[2];
	pid_t serverpid;
}lsp_server;

lsp_server start_lsp_server(int port){
	lsp_server server;
	memset(&server, 0,sizeof(lsp_server));
	
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
	if(pipe( server.inboxfds ) < 0){ perror("cant create inbox pipes")};
	if(pipe( server.outboxfds ) < 0){ perror("cant create outbox pipes")};

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
		close(server.inbox[0]);
		close(server.outbox[1]);
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
		/*
			Clean up file descriptors, close read end of inbox and write end of outbox
		*/
		close(server.inbox[1]);
		close(server.outbox[0]);
	
		struct sockaddr_in clientaddr; 	 /* Internet client address */
		int clientaddrlen = sizeof(clientaddr);
		char buffer[BUFFER_LENGTH];
		linked_packet inbox;
		linked_packet outbox;
	
		fd_set readfds, afds;
		int nfds = getdtablesize();
		FD_ZERO(&afds);  
		FD_SET(server.socketfd, &afds);
		FD_SET(server.outboxfds[1], &afds);
	

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
				//TODO put message into server.inbox[0]
			
			}
			if(FD_ISSET(server.outbox[1], &readfds)){
				//TODO read from outbox into buffer
				//TODO build packet from buffer
				//TODO send packet with  if(sendto(server.socketfd, ....
			}
		}
	}
}
