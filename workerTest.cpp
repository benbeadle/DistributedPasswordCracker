#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <time.h>
#include <math.h>
#include <sstream>
#include <vector>
#include "lsp.h"

//For sleep function
#include <dos.h>
#include <conio.h>

using namespace std;

bool signal_exit = false;
void ctrlExit(int signum) {
	signal_exit = true;
}

char* increase(char* input, int len) {
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
	return input;
}

void splitArgs(const char* input, char* one, char* two) {
    int len = (strchr(input, ':') - input) * sizeof(char);
    strncpy(one, input, len);
    strcpy(two, input + len + 1);
}

vector<string> splitString(string input) { 
	stringstream ss(input);
	string token;
	vector<string> returnStr;
	while (ss >> token)
	{
		returnStr.push_back(token);
	}
	return returnStr;
}

int shaTest(char* input, int len, char* pass) {
	int i = 0;
	unsigned char temp[SHA_DIGEST_LENGTH];
	char buf[SHA_DIGEST_LENGTH*2];

	memset(buf, 0x0, SHA_DIGEST_LENGTH*2);
	memset(temp, 0x0, SHA_DIGEST_LENGTH);

	SHA1((unsigned char *)input, len, temp);

	for (i=0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf((char*)&(buf[i*2]), "%02x", temp[i]);
	}
	return strcmp(buf,pass)==0;
}


int main(int argc, char* argv[]) {
	signal(SIGABRT,ctrlExit);
	signal(SIGTERM,ctrlExit);
	//signal(SIGINT, ctrlHandler);
	//signal(SIGKILL, ctrlHandler);
	
	if (argc != 2) {
        printf("Invalid parameters. Must be of form: host:port");
        return 0;
    }
	
    uint8_t* pld;		//The message payload
    char* destination;	//The server destination
    char* port;			//The server port
    int bytes;			//Number of bytes read from server
	
	//Split the input arguments
	splitArgs(argv[0], destination, port);
	
	
    lsp_client * worker = lsp_client_create(destination, (int)port);
	
	while(true){
		//Wait 5 minutes
		sleep(300000);
	}
	
	//lsp_client_close(worker);
	cout << "Goodbye! - WORKER TEST" << endl;
}
