#include <iostream>
#include <queue>
#include <string>
#include <list>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <sstream>
#include <vector>
#include "lsp.h"

using namespace std;

bool signal_exit = false;
void ctrlExit(int signum) {
	signal_exit = true;
}

int main(int argc, char* argv[]) {
	signal(SIGABRT,ctrlExit);
	signal(SIGTERM,ctrlExit);
	
	//Create the variables
	//TODO: Check for valid arguments
	void* pld;
	int port = int(argv[0]);
	
	lsp_server* server = lsp_server_create(port);
  
  while(true){}
}