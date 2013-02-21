#ifndef QUEUE_H
#define QUEUE_H
#pragma once
#include "lspmessage.pb-c.h"
#include "lsp_sever.c"

typedef struct queue_node{
	LSPMessage* msg;
	struct queue_node* next;
}queue_node;

void push_back(LSPMessage* msg, queue_node* head);

/*
	if there are any packets in the inbox then msg will point to the next packet and zero returned. 
	Else -1 will be returned
*/
int consume_next(LSPMessage* msg, queue_node* head);

void free_queue(queue_node* head);

#endif
