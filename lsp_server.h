#ifndef LSP_SERVER_H
#define LSP_SERVER_H
#pragma once
#include "queue.h"
#include "lspmessage.pb-c.h"
#include <netinet/in.h>
#include <signal.h>

#define BUFFER_LENGTH 100
#define FALSE 0
#define SET_EPOCH_CNT 0
#define SET_EPOCH_LNTH 1
#define SET_DROP_RATE 2
#define DROP_CONN 3
#define BILLION 1000000000L
#define _EPOCH_LTH 2.0
#define _EPOCH_CNT 5
#define _DROP_RATE 0.0

/************************
			server structs
**************************/


typedef struct{
    int socketfd;
    int port;
    int outboxfd[2]; //from application to LSP
    int inboxfd[2]; //from LSP to application
	int cmdpipefd[2];
	queue_node* inbox_queue;
}lsp_server, lsp_client, lsp_user;

/************************
			CSM structs
**************************/

typedef enum{
    wait_to_send,
    wait_to_receive
}State;

typedef struct client_state_machine{
    LSPMessage* latest_ACK_sent; //The latest message sent, keep sending the ack
    State current_state;
    LSPMessage* latest_message_sent;
    int missed_epochs;
    int latest_epoch_seq;
    int connid;
	int last_seqnum_used;
    struct sockaddr_in clientaddr;
    queue_node* outbox_queue;
}client_state_machine;

/************************
			lsp user directory structs
**************************/
typedef struct lsp_user_node{
	lsp_user* lspu;
	struct lsp_user_node* next;
}lsp_user_node;

/************************
			client_registry structs
**************************/

typedef struct client_registry_node{
	client_state_machine* csm;
	struct client_registry_node* next;
}client_registry_node;

/************************
			server Functions
**************************/

lsp_server* start_lsp_server(int port);
LSPMessage* read_from_pipe(const int pipefd);
int send_through_pipe(LSPMessage* msg, const int pipefd);
int send_packet(LSPMessage* msg, const struct sockaddr_in* clientaddr, const int socket, socklen_t clientaddrlen);
LSPMessage* recieve_packet(const int socket, struct sockaddr* clientaddr, socklen_t clietaddrlen);
int get_next_connectionId();
void epoch_tick(client_state_machine* csm);
void sigepoch_hdl(int sig, siginfo_t* info, void* parms);
void sigterm_hdl(int sig);
void change_epoch_limit(int times);
void change_drop_rate(double new_rate);
void free_lsp_user( lsp_user* lspu);



/************************
			CSM Functions
**************************/


void initialize_csm(client_state_machine* csm,  const struct sockaddr_in clientaddr, lsp_server* server);
void free_csm(client_state_machine* csm);
LSPMessage* createACK(const int connid, const int seqnum);
void send_msg(LSPMessage* message, client_state_machine* csm, lsp_server* server);
void wts_to_wtr(client_state_machine* csm, lsp_server* server);
void wtr_to_wts(client_state_machine* csm, lsp_server* server);
void receive_msg(LSPMessage* message, client_state_machine* csm, lsp_server* server);


/*****
	EPOCH FUNCTIONS

*****/


void interrupt(int signo,  siginfo_t *info, void *context);
int setinterrupt();
int setperiodic(double sec);
int changeperiodic(double sec);
timer_t timerid;

/************************
			lsp user directory functions
**************************/

/*
	if found, the lspu pointer will be populated with the state machine and 0 will be returned. 
	Else, a -1 will be returned
*/
int find_by_port(lsp_user_node* reg, const int port, lsp_user* lspu);

//removes the specified lsp_user from the directory and returns the new pointer to the registry
lsp_user_node* remove_by_port(lsp_user_node* reg, const int port);
	
//applies a function to all members of the registry.
void apply_to_all ( lsp_user_node* reg, void (*f)(lsp_user*) );


/************************
		 client_registry functions
**************************/


/*
	if found, the csm pointer will be populated with the state machine and 0 will be returned. 
	Else, a -1 will be returned
*/
int find_by_connid(client_registry_node* reg, const uint32_t connid, client_state_machine* csm);
	
//if found, the csm pointer will be populated with the state machine and 0 will be returned. Else, a -1 will be returned
int find_by_clientaddr(client_registry_node* reg, const struct sockaddr_in clientaddr, client_state_machine* csm);

//removes the csm from the registry, and returns the pointer to the new registry head.
client_registry_node* remove_by_connid(client_registry_node* reg, uint32_t connid);

#endif
