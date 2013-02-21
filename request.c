#include <stdio.h>
#include <stdint.h>
#include "lsp.h"

void split(const char* input, char* dest, char* port) {
    int len = (strchr(input, ':') - input) * sizeof(char);
    strncpy(dest, input, len);
    strcpy(port, input + len + 1);
}


int main(int argc, char * argv[]) {
    if (argc != 4) {
        printf("Invalid parameters (%d). Must be of form: host:port hash len\n", argc);
        return 0;
    }
	
    uint8_t* pld;		//The message payload
    char* destination;	//The server destination
    char* port_char;
    int port;			//The server port
    int bytes;			//Number of bytes read from server
	
	//Split the input arguments
	split(argv[0], destination, port_char);
	
  port = *port_char - '0';
  
  lsp_client * requester = lsp_client_create(destination, port);

	//Make sure it successfully connected to the server
    if (requester == NULL) {
        printf("Unable to connect to server.");
        return 0;
    }
	
	//Write to the client the hash and the password length
    int success = lsp_client_write(requester, (uint8_t*)argv[1], atoi(argv[2]));
    if (success == 0) {
        printf("Unable to write to server");
		lsp_client_close(requester);
        return 0;
    }
	
	//Keep looping until bytes read is not 0
	//-1 means the connection was lost
	//Greater than 0 means we read something in
    while (bytes = lsp_client_read(requester, pld) == 0) {
        sleep(1000);
    }
	
	//Print out the corresponding result
    if (bytes <= -1) {
        printf("Disconnected");
    } else {
		char* res = (char*)pld;
        if (res[0] == 'f') {
			memmove(res, res+1, strlen (res+1));
            printf("Found: %s", res);
        } else {
            printf("Not Found");
        }
    }
	
    lsp_client_close(requester);
    return 0;
}
