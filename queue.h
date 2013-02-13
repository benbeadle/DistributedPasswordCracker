#ifndef QUEUE_H
#define QUEUE_H

typedef struct{
	LSPMessage* msg;
	queue_node* next;
}queue_node;

void push_back(LSPMessage* msg, queue_node* head);

/*
	if there are any packets in the inbox then msg will point to the next packet and zero returned. 
	Else -1 will be returned
*/
int consume_next(LSPMessage* msg, queue_node* head);

#endif
