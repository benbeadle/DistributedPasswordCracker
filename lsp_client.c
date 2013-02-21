#include "lsp_server.h"
#include <stdlib.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

lsp_client* client = server; //Short and easy way for me to take care of compatibility issues      

lsp_client* start_lsp_client(const char* dest, int port){
	srand(12345);
	
	//get server name
	char server_name[256];
	strcpy(server_name, dest);

    //allocate client struct
	client = static_cast<lsp_client*>(malloc(sizeof(lsp_client)));
    memset(client, 0, sizeof(lsp_client));

	client->port = port;
	
	//Setup server address
	struct hostent *hostp;
	sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(server_name);
	
	//Check server address
	if (serveraddr.sin_addr.s_addr == (unsigned long) INADDR_NONE)       {

          hostp = gethostbyname(server_name);  // get the IP address from the dotted decimal
          if (hostp == (struct hostent *) NULL)   {
              printf("ERROR Host not found --> ");
	       return NULL;
           }
		memcpy(&serveraddr.sin_addr,  hostp->h_addr, sizeof(serveraddr.sin_addr));
    }

    
    /* Allocate socket */
    client->socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client->socketfd < 0) perror("can’t create socket");
    
    // set SO_REUSEADDR so that we can re-use not fully deallocated chatrooms
    int optval = 1;
    setsockopt(client->socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    
    /* Bind the socket */
    if (bind(client->socketfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        perror("can’t bind");
    
    /*create pipes*/
    if(pipe(client->outboxfd) < 0) perror("cant create outbox pipe");
    if(pipe(client->inboxfd) <0) perror("cant create outbox pipe");
    if(pipe(client->cmdpipefd) <0) perror("cant create cmd pipe");

    int clientpid = fork();
    if(clientpid == -1){
        perror("failed to fork");
    } else if (clientpid != 0){
        /*
            We are the parent, clean up and return
        */
        //close the socket, we wont use it
        close(client->socketfd);
        //close write end of inbox and read end of outbox
        close(client->inboxfd[0]);
        close(client->outboxfd[1]);
		//close the read end of the cmd pipe
		close(client->cmdpipefd[1]);
        
        return client;

    } else{
        /*
        We are the child
        */
		//close read end of inbox and write end of outbox
        close(client->inboxfd[1]);
        close(client->outboxfd[0]);
		//close write end of cmd pipe
		close(client->cmdpipefd[0]);
		//initialize some things we need	
        //struct sockaddr_in clientaddr;   /* Internet client address */
        //socklen_t clientaddrlen = (socklen_t) sizeof(clientaddr);
		socklen_t serveraddrlen = (socklen_t) sizeof(serveraddr);
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
			Start up a CSM and tell the server we're here and initiate the connection
			we are keeping this in a client registry so that I could re-use the server code
		*/
		client_registry_node* client_registry = static_cast<client_registry_node*>(malloc(sizeof(client_registry_node)));
		client_registry->csm = static_cast<client_state_machine*>(malloc(sizeof(client_state_machine)));
		initialize_csm(client_registry->csm, serveraddr, client); //this will send the initialization ACK
		client_registry->csm->current_state = wait_to_receive; //We need to wait for their ACK

		
		/*
			Get ready to enter select statement
		*/
        fd_set readfds, writefds, rcopyfds, wcopyfds;
        int nfds = getdtablesize();
        FD_ZERO(&readfds);  
        FD_ZERO(&writefds);  
        FD_ZERO(&rcopyfds);  
        FD_ZERO(&wcopyfds);  
        FD_SET(client->socketfd, &rcopyfds);
        FD_SET(client->outboxfd[1], &rcopyfds);
		FD_SET(client->cmdpipefd[0], &rcopyfds);
		FD_SET(client->inboxfd[0], &wcopyfds);

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
            if(FD_ISSET(client->socketfd, &readfds)) {
                msg = recieve_packet(client->socketfd, (sockaddr*) &serveraddr, serveraddrlen);
				if(((double)rand()/(double)RAND_MAX) < drop_rate ){ //see if we should just drop the packet for paramaterized drop rate
					lspmessage__free_unpacked( msg, NULL);
				}
				else if(msg->seqnum == 0){ //we've established our connection
					printf("New connection ACKed (you should only see this once");
					client_registry->csm->connid = msg->connid;
					receive_msg(msg, client_registry->csm, client);
					
				}
                else{ //Else find the csm with that address and call recieve_msg(csm)		
					if(find_by_clientaddr(client_registry, serveraddr, csm) < 0){
						printf("WARNING: unable to find client for connection, dumping recieved packet");
						lspmessage__free_unpacked( msg, NULL);
					} else receive_msg(msg, csm, client);
				}
            }
            if(FD_ISSET(client->inboxfd[0], &writefds)){ //The main program wants the next message which is in the server's single inbox_list linked list.
				consume_next(msg, client->inbox_queue);
				send_through_pipe(msg, client->inboxfd[0]);
				lspmessage__free_unpacked(msg, NULL);
			}
            if(FD_ISSET(client->outboxfd[1], &readfds)){ //The main program want to send out a message
                msg = read_from_pipe(client->outboxfd[1]);
				msg->connid = client_registry->csm->connid;
				send_msg(msg, csm, client);
            }
			if(FD_ISSET(client->cmdpipefd[1], &readfds)){ //The main program wants to send a command
				/*
					To make my life easier, we are sending these commands as LSPMessages. connid represents the type of command,
					seqnum the parameter. Doubles will be sent as an int, and divided by 100.
					
				*/
				int i;
				double d;
				msg = read_from_pipe(client->cmdpipefd[1]);
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
