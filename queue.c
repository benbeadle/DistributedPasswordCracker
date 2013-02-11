#include "queue.h"

void push_back(LSPMessage* msg, queue_node* head){
	queue_node* node = malloc(sizeof(queue_node));
	node->msg = msg;
	node->next = head;
	head = node;
}