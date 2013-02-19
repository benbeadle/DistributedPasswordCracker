#ifndef LSP_H
#define LSP_H

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <cstdio>
//#include "client_state_machine.h"
#include "queue.h"
#include "lsp_server.h"
#include "lspmessage.pb-c.h"

// Global Parameters. For both server and clients.





void lsp_set_epoch_lth(double lth);
void lsp_set_epoch_cnt(int cnt);
void lsp_set_drop_rate(double rate);
void epoch_lth_func(lsp_user*);
void set_epoch_cnt_func(lsp_user*);
void set_drop_func(lsp_user*);

lsp_client* lsp_client_create(const char* src, int port);
int lsp_client_read(lsp_client* a_client, uint8_t* pld);
bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth);
bool lsp_client_close(lsp_client* a_client);


lsp_server* lsp_server_create(int port);
int lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id);
bool lsp_server_write(lsp_server* a_srv, void* pld, int lth, uint32_t conn_id);
bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id);

#endif
