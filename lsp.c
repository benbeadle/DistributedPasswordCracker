#include "lsp.h"
#include "lspmessage.pb-c.h"

double epoch_lth = _EPOCH_LTH;
int epoch_cnt = _EPOCH_CNT;
double drop_rate = _DROP_RATE;
lsp_user_node* registry;

/*
 *				LSP RELATED FUNCTIONS
*				Each function is design to kick-off the passing 
				of the approprate function pointer to all LSPusers in the registry
 */  

void lsp_set_epoch_lth(double lth){
	epoch_lth = lth;
	if(registry != NULL){
		apply_to_all(registry, epoch_lth_func);
	}
	
}
void epoch_lth_func(lsp_user*){
	uint8_t* buf;
	int ilth = (int) (epoch_lth * 100);
	int len;
	
	LSPMessage msg = LSPMESSAGE__INIT;
	msg.connid = SET_EPOCH_LNTH;
	msg.seqnum = ilth;
	msg.payload.len = 0;
	
	len = lspmessage__get_packet_size(&msg);
	buf = malloc(len)
	lspmessage__pack(&msg, buf);
	
	if( write(lsp_user->cmdpipefd[0], buf, len) < 0){
		perror("Failed sending epch_lth to LSP item");
	}
	
	free(buf);
}
void lsp_set_epoch_cnt(int cnt){
	epoch_cnt = cnt;
	if(registry != NULL){
		apply_to_all(registry, set_epoch_cnt_func);
	}
}
void set_epoch_cnt_func(lsp_user*){
	uint8_t* buf;
	int len;
	
	LSPMessage msg = LSPMESSAGE__INIT;
	msg.connid = SET_EPOCH_CNT;
	msg.seqnum = epoch_cnt;
	msg.payload.len = 0;
	
	len = lspmessage__get_packet_size(&msg);
	buf = malloc(len)
	lspmessage__pack(&msg, buf);
	
	if( write(lsp_user->cmdpipefd[0], buf, len) < 0){
		perror("Failed sending cnt_lth to LSP item");
	}
	
	free(buf);
}

void lsp_set_drop_rate(double rate){
	drop_rate = rate;
	if(registry != NULL){
		apply_to_all(registry, set_drop_func);
	}
}
void set_drop_func(lsp_user*){
	uint8_t* buf;
	int irate = (int) (drop_rate * 100);
	int len;
	
	LSPMessage msg = LSPMESSAGE__INIT;
	msg.connid = SET_DROP_RATE;
	msg.seqnum = irate;
	msg.payload.len = 0;
	
	len = lspmessage__get_packet_size(&msg);
	buf = malloc(len)
	lspmessage__pack(&msg, buf);
	
	if( write(lsp_user->cmdpipefd[0], buf, len) < 0){
		perror("Failed sending drop_rate to LSP item");
	}
	
	free(buf);
}

/*
 *				CLIENT RELATED FUNCTIONS
 *				
 */  


//Returns false if server is not available
lsp_client* lsp_client_create(const char* src, int port)
{
	lsp_client* c = new lsp_client(src, port);
	
	//add client to registry
	lsp_user_node* node = malloc(sizeof(lsp_user_node));
	node->lsp_user = c;
	node->next = registry;
	registry = node;
	
	return c;
}

//Returns -1 when connection lost
//Returns # bites read
int lsp_client_read(lsp_client* a_client, uint8_t* pld)
{
	LSPMessage* msg;

	// Read packed message from standard-input.
	uint8_t buf[BUFFER_LENGTH];
	size_t msg_len = read(a_client->inbox[1], buf, BUFFER_LENGTH);
	if(msg_len < 0){
		perror("cant recieve server");
		return -1;
	}

	// Unpack the message using protobuf-c.
	msg = lspmessage__unpack(NULL, msg_len, buf);   
	if (msg == NULL){
	  fprintf(stderr, "error unpacking incoming message from client\n");
	  exit(1);
	}
	
	memcpy(pld, msg->payload.data, msg->payload.len);
	msg_len = msg->payload.len;
	
	lsp_free_msg(msg, NULL);
	return msg_len;
}

//Should not send null
bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth)
{
	uint8_t* buf;
	int irate = (int) (drop_rate * 100);
	int len;
	
	LSPMessage msg = LSPMESSAGE__INIT;
	msg.connid = conn_id;
	msg.seqnum = 0;
	msg.payload.data = malloc(sizeof(uint8_t)* lth);
	msg.payload.len = lth;
	memcpy(msg.payload.data, pld, lth*sizeof(uint*_t));
	
	len = lspmessage__get_packet_size(&msg);
	buf = malloc(len)
	lspmessage__pack(&msg, buf);
	
	if( write(a_client->outboxfd[0], buf, len) < 0){
		perror("Failed writing to LSP client");
	}
	
	free(buf);
	free(msg.payload.data);
}

//Close connection, remember free memory
bool lsp_client_close(lsp_client* a_client)
{
	//Since the desctuctor automatically gets called,
	//Don't think we need to do anything here
}

/*
 *				SERVER RELATED FUNCTIONS
 */  


lsp_server* lsp_server_create(int port)
{
	lsp_server* server = new lsp_client(src, port);

	//add server to registry
	lsp_user_node* node = malloc(sizeof(lsp_user_node));
	node->lsp_user = server;
	node->next = registry;
	registry = node;
	
	return server
}

int lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id)
{
	LSPMessage* msg;

	// Read packed message from standard-input.
	uint8_t buf[BUFFER_LENGTH];
	size_t msg_len = read(a_srv->inbox[1], buf, BUFFER_LENGTH);
	if(msg_len < 0){
		perror("cant recieve server");
		return -1;
	}

	// Unpack the message using protobuf-c.
	msg = lspmessage__unpack(NULL, msg_len, buf);   
	if (msg == NULL){
	  fprintf(stderr, "error unpacking incoming message\n");
	  exit(1);
	}
	
	memcpy(pld, msg->payload.data, msg->payload.len);
	memcpy(connid, msg->connid, sizeof(uint32_t));
	msg_len = msg->payload.len;
	
	lsp_free_msg(msg, NULL);
	return msg_len;
}

bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, uint32_t conn_id)
{
	uint8_t* buf;
	int irate = (int) (drop_rate * 100);
	int len;
	
	LSPMessage msg = LSPMESSAGE__INIT;
	msg.connid = conn_id;
	msg.seqnum = 0;
	msg.payload.data = malloc(sizeof(uint8_t)* lth);
	msg.payload.len = lth;
	memcpy(msg.payload.data, pld, lth*sizeof(uint*_t));
	
	len = lspmessage__get_packet_size(&msg);
	buf = malloc(len)
	lspmessage__pack(&msg, buf);
	
	if( write(a_srv->outboxfd[0], buf, len) < 0){
		perror("Failed sending epch_lth to LSP server");
	}
	
	free(buf);
	free(msg.payload.data);
}

bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id)
{
	uint8_t* buf;
	int len;
	
	LSPMessage msg = LSPMESSAGE__INIT;
	msg.connid = SET_DROP_RATE;
	msg.seqnum = conn_id; //CMD packets have different structure. Deal with it
	msg.payload.len = 0;
	
	len = lspmessage__get_packet_size(&msg);
	buf = malloc(len)
	lspmessage__pack(&msg, buf);
	
	if( write(a_srv->cmdpipefd[0], buf, len) < 0){
		perror("Failed sending close to LSP server");
	}
	
	free(buf);
}