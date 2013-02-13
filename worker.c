#include <signal.h>
#include <stdio.h>
#include <stdint.h>
// Received packet: c hash lower upper
// Sent packet: x hash lower upper || f pass

typedef int bool;
#define true 1
#define false 0

bool keepRunning = true;
void ctrlHandler(int dummy=0) {
    keepRunning = false;
}

typedef struct {

} lsp_client;

lsp_client* lsp_client_create(const char* dest, int port){
	lsp_client* c; 
	return c;
}
int lsp_client_read(lsp_client* a_client, uint8_t* pld){return 1;}
bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth){return false;}
bool lsp_client_close(lsp_client* a_client){return false;}

//Increase the string. Ex: aaaa -> aaab
void increase(char* input, int len) {
	input[len - 1]++;
	int i;
	for(i = len-1; i >= 0; i--) {
		if(input[i] == '{') {
			input[i] = 'a';
			if(i - 1 >= 0) {
				input[i - 1]++;
			}
		}
	}
	printf("%s\n", input);
}

void split(const char* input, char* one, char* two) {
    int len = (strchr(input, ':') - input) * sizeof(char);
    strncpy(one, input, len);
    strcpy(two, input + len + 1);
}

int main(int argc, char *argv[])
{
	if (argc != 1 || strchr(argv[0], ':') == NULL) {
        printf("Invalid parameters. Must be of form: host:port hash len");
        return 0;
    }
	
    uint8_t* pld;		    //The message payload
    char* destination;	//The server destination
    int port;			      //The server port
    int bytes;			    //Number of bytes read from server
	
	//Split the input arguments
	split(argv[0], destination, port);
	
    lsp_client * worker = lsp_client_create(destination, port);

	//Make sure it successfully connected to the server
  if (worker == NULL) {
      printf("Unable to connect to server.");
      return 0;
  }
	
	//TODO create join message to send
	uint8_t* join = 'j';
	int success = lsp_client_write(worker, join, sizeOf(*uint8_t));
	if (success == 0) {
        printf("Unable to send join message to server.");
		lsp_client_close(worker);
        return 0;
	}
	
	signal(SIGINT, ctrlHandler);
	signal(SIGKILL, ctrlHandler);
	
	while(keepRunning) {
		int bytes;
		while (bytes = lsp_client_read(requester, pld) == 0) {
			sleep(1000);
		}
		
		if (bytes <= -1) {
			printf("Disconnected");
		}
		
		//Remove the c and space (assuming valid message format)
		memmove (pld, pld+1, strlen (pld+1));
		memmove (pld, pld+1, strlen (pld+1));
		
		//Call split twice to get the hash, lower, and upper values
		//Also get the length of the lower to know how many letters
		char* hash;
		char* lower;
		char* upper;
		int len = 0;
		split(pld, hash, lower);
		split(lower, lower, upper);
		len = strlen(lower);
		
		// TODO Call SHA1 in loop for range given
		
		//TODO Save result in int
		uint8_t* result;
		success = lsp_client_write(worker, result, sizeOf(*result));
		if (success == 0) {
			cout << "ERROR: Write Failed
		}
	}
	
	lsp_client_close(worker);
	
	return 0;
}