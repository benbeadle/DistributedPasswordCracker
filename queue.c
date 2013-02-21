#include "queue.h"
#include <cstdlib>
#include <stdio.h>

void push_back(LSPMessage* msg, queue_node* head){
	queue_node* node = static_cast<queue_node*>(malloc(sizeof(queue_node)));
	node->msg = msg;
	node->next = head;
	head = node;
}

int consume_next(LSPMessage* msg, queue_node* head){
	fprintf(stderr, "%s consume_next called \n",TAG);
	if(head == NULL) { //no messages in the inbox
		fprintf(stderr, "%s consum_next no messages in inbox \n",TAG);
		msg = NULL;
		return -1;
	} 
	else if(head->next == NULL){ //one message in the inbox
		msg = head->msg;
		free(head); //we no longer need this message
		return 0;
	} 
	else{
		queue_node* head_cpy = head; //copy head, as we will still have messages left afterwards
		while(head_cpy->next->next != NULL){ //Find the second to last node
			head_cpy = head_cpy->next;
		}
		msg = head_cpy->next->msg; //msg is the last node
		free(head_cpy->next); //free the last node
		return 0;
	}
}

void free_queue(queue_node* head){
	if(head->next != NULL){
		free_queue(head->next);
	}
	lspmessage__free_unpacked(head->msg, NULL);
	free(head);
	return;
}
