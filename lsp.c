#include "lsp.h"
#include "lspmessage.pb-c.h"

double epoch_lth = _EPOCH_LTH;
int epoch_cnt = _EPOCH_CNT;
double drop_rate = _DROP_RATE;

/*
 *
 *
 *				LSP RELATED FUNCTIONS
 *
 *
 */  

void lsp_set_epoch_lth(double lth){epoch_lth = lth;}
void lsp_set_epoch_cnt(int cnt){epoch_cnt = cnt;}
void lsp_set_drop_rate(double rate){drop_rate = rate;}


/*
 *
 *
 *				CLIENT RELATED FUNCTIONS
 *
 *
 */  


//Returns false if server is not available
lsp_client* lsp_client_create(const char* src, int port)
{
	lsp_client* c = new lsp_client(src, port);
	if(c->sd < 0)
		return 0;
	return c;
}

//Returns -1 when connection lost
//Returns # bites read
int lsp_client_read(lsp_client* a_client, uint8_t* pld)
{
	return lsp_client->read(pld);
}

//Should not send null
bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth)
{
	return a_client->write(pld, lth);
}

//Close connection, remember free memory
bool lsp_client_close(lsp_client* a_client)
{
	//Since the desctuctor automatically gets called,
	//Don't think we need to do anything here
}

/*
 *
 *
 *				SERVER RELATED FUNCTIONS
 *
 *
 */  


lsp_server* lsp_server_create(int port)
{

}

int lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id)
{

}

bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, uint32_t conn_id)
{

}

bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id)
{

}