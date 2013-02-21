#include lsp.h
#define BUFFER_LENGTH 100
#define FALSE 0

typedef struct{
	int socketfd;
	struct sockaddr_in serveraddr;          /* Internet endpoint address */
	pthread_t server_thread;
	linked_packet inbox_head;
	linked_packet outbox_head;
}lsp_server;

typedef struct{
	lsp_packet packet;
	lsp_packet* next;
}linked_packet;

lsp_server start_lsp_server(int port){
	lsp_server server;
	memset(&server, 0,sizeof(lsp_server));
	
	
	server.serveraddr.serveraddr_family      = AF_INET;
	server.serveraddr.sin_port				  = htons(port);
	server.serveraddr.serveraddr_addr.s_addr = htonl(INADDR_ANY);
	int s = server.socketfd;
	
	/* Allocate socket */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) perror("can’t create socket");
	
	// set SO_REUSEADDR so that we can re-use not fully deallocated chatrooms
	int optval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	
	/* Bind the socket */
	if (bind(s, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
		perror("can’t bind");
	
	if(pthread_create(&(server.server_thread), NULL, server_multiplexing, &server){
			perror("Error creating thread");
	}
	return lsp_server;
}
void *server_multiplexing(void *server_void_ptr){
	lsp_server *server = (lsp_server *) lsp_void_ptr;
	
	struct sockaddr_in clientaddr; 	 /* Internet client address */
	int clientaddrlen = sizeof(clientaddr);
	char buffer[BUFFER_LENGTH];
	int sockfd = server->socketfd;
	
	fd_set readfds, afds;
	int nfds = getdtablesize();
	FD_ZERO(&afds);  FD_SET(sockfd, &afds);
	

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
		if(FD_ISSET(sockfd, &readfds)){
			int rc = recv(s_sock, &buffer, BUFFER_LENGTH, 0, server->serveraddr, &clientaddr, &clientaddrlen);
			if(rc < 0){
				perror("recv()");
			}
			
			lsp_packet packet* = malloc(sizeof(lsp_packet));
			//TODO build packet from buffer (probs use protobufs to de-serialize)
			//TODO place packet at end of server->inbox
			
		}
	}
	return NULL;

}
