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

const int JOBSIZE = 1000;

bool signal_exit = false;
void ctrlExit(int signum) {
	signal_exit = true;
}

string toString(int i) {
	ostringstream oss;
	oss << i;
	return oss.str();
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

class SubJob {
	private:
		string beginning;
		int len;
	
	public:
		SubJob(string b, int l) {
			beginning = b;
			len = l;
		}
		string getBegin() {
			return beginning;
		}
		int getLen() {
			return len;
		}
};

class Job {
	
	private:
		string hash;
		int len;
		uint32_t* conn_id;
		list<SubJob> subjobs;
		list<SubJob>::iterator current_sub;
	public:
		Job(string h, int l, uint32_t* c) {
			hash = h;
			len = l;
			conn_id = c;
			
			double total_jobs = pow(26.0, l);
			int number_jobs = ceil(total_jobs / JOBSIZE);
			string current_string = "a";
			for(int i = 1; i < l; i++)
				current_string += "a";
			
			for(int i = 0; i < number_jobs; i++) {
				SubJob add = SubJob(current_string, JOBSIZE);
				current_string = string(increase((char*)current_string.c_str(), JOBSIZE));
				subjobs.push_back(add);
			}
			
			current_sub = subjobs.begin();
		}
		
		bool isNULL() {
			return (hash == "");
		}
		
		void setNULL() {
			hash = "";
			len = 0;
			conn_id = 0;
		}
		
		SubJob nextJob() {
			SubJob return_sub = *current_sub;
			
			if(current_sub == subjobs.end())
				current_sub = subjobs.begin();
			else
				current_sub++;
			
			return return_sub;
		}
		int removeSub(string input) {
			list<SubJob>::iterator iter;
			for(iter = subjobs.begin(); iter != subjobs.end(); iter++) {
				if(input == (*iter).getBegin()) {
					if(current_sub == iter)
						current_sub++;
					subjobs.erase(iter);
					return subjobs.size();
				}
			}
			return subjobs.size();
		}
		string getHash() {
			return hash;
		}
		uint32_t* getConnId() {
			return conn_id;
		}
};

int main(int argc, char* argv[]) {
	signal(SIGABRT,ctrlExit);
	signal(SIGTERM,ctrlExit);
	
	//Create the variables
	//TODO: Check for valid arguments
	void* pld;
	int port = int(argv[0]);
	queue<Job> waiting_jobs;
	queue<uint32_t*> idle_workers;
	Job current_job = Job("", 0, 0);
	
	lsp_server* server = lsp_server_create(port);
	
	if(server == NULL) {
		cout << "Unable to start server on port " << port << "." << endl;
		return 0;
	}
	
	while(!signal_exit) {
		uint32_t* conn_id;
		int size = lsp_server_read(server, pld, conn_id);
		
		if(size > 0) {
			char* readc = reinterpret_cast<char*>(pld);
			string read = readc;
			
			char command = read[0];
			
			if(command == 'j') { //Worker Joining
				if(current_job.isNULL()) {
					idle_workers.push(conn_id);
				} else {
					SubJob n = current_job.nextJob();
					string message = "c " + current_job.getHash() + " " + n.getBegin() + " " + toString(n.getLen());
					lsp_server_write(server, &message[0], sizeof(message), *conn_id);
				}
			} else if(command == 'c') { //Crack Request
				vector<string> split = splitString(read);
				
				Job newJob = Job(split[1], int(atoi(split[2].c_str())), conn_id);
				
				if(current_job.isNULL()) {
					current_job = newJob;
					while(idle_workers.size() != 0) {
						uint32_t* idle_worker = idle_workers.front();
						idle_workers.pop();
						SubJob next_job = current_job.nextJob();
						string message = "c " + current_job.getHash() + " " + next_job.getBegin() + " " + toString(next_job.getLen());
						lsp_server_write(server, (void*)(message.c_str()), sizeof(message), *idle_worker);
					}
				} else {
					waiting_jobs.push(newJob);
				}
				
			} else if(command == 'f') { //Password Found!!
				lsp_server_write(server, ((void*)read.c_str()), sizeof(read), *current_job.getConnId());
				current_job.setNULL();
				if(waiting_jobs.size() != 0) {
					current_job = waiting_jobs.front();
					SubJob n = current_job.nextJob();
					string message = "c " + current_job.getHash() + " " + n.getBegin() + " " + toString(n.getLen());
					lsp_server_write(server, (void*)(message.c_str()), sizeof(message), *conn_id);
				} else {
					idle_workers.push(conn_id);
				}
			} else if (command == 'x') { //No password found
				int count = current_job.removeSub(splitString(read)[1]);
				
				if(count == 0) {
					string message = "x";
					lsp_server_write(server, (void*)(message.c_str()), sizeof(message), *current_job.getConnId());
					current_job.setNULL();
					if(waiting_jobs.size() != 0) {
						current_job = waiting_jobs.front();
						waiting_jobs.pop();
					}
				}
				
				if(!current_job.isNULL()) {
					SubJob n = current_job.nextJob();
					string message = "c " + current_job.getHash() + " " + n.getBegin() + " " + toString(n.getLen());
					lsp_server_write(server, ((void*)message.c_str()), sizeof(message), *conn_id);
					current_job.setNULL();
				} else {
					idle_workers.push(conn_id);
				}
				
			} else {
				cout << "Invalid message: " << read << endl;
			}
			
		} else if(size == -1) {
			cout << "Connection lost." << endl;
			signal_exit = true;
		}
	}
	
	while(idle_workers.size() != 0) {
		lsp_server_close(server, *idle_workers.front());
		idle_workers.pop();
	}
	
	cout << "Thanks for using our Distributed Password Cracker!" << endl;
	
	return 0;
}
