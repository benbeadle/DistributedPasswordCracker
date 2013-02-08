#include "lsp_client.h"

/* Variables:


inbox/outbox (queue/LL of packets)
*/

struct sockaddr_in serveraddr;
socklen_t serveraddrlen = sizeof(serveraddr);
struct hostent *hostp;

typedef struct {
	uint32_t id;
	uint32_t sn;
	uint8_t* d[];
	
} lsp_packet;

class lsp_client{
	
	lsp_packet* indbox_head;
	lsp_packet* outbox_head;
	int sequence_number;
	
	public:
	int sd;
	
	//Constructor method
	//Return false if the server is not available
	lsp_client(const char* src, int port) {
		sd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sd >= 0) {
			memset(&serveraddr, 0, sizeof(serveraddr));
			serveraddr.sin_family      = AF_INET;
			serveraddr.sin_port        = htons(port);
			serveraddr.sin_addr.s_addr = inet_addr(src);
			
			if (serveraddr.sin_addr.s_addr == (unsigned long) INADDR_NONE) {
				hostp = gethostbyname(src);  // get the IP address from the dotted decimal
				if (hostp != (struct hostent *) NULL) {
					memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
				}
			}
			
			//Set the first sequence number
			sequence_number = 1;
		}
	}
	
	//Destructor method (close)
	~lsp_client() {
		//-1 means it couldn't create it
		if(sd != -1)
			close(sd);
	}
	
	//Read from the server
	//Return -1 when connection lost
	//Return number of bytes read
	int read(uint8_t* pld) {
		//TODO: MARSHALING
		
		if(inbox_
		
		int l = recvfrom(sd, pld, sizeof(pld), 0, (struct sockaddr *)&serveraddr, & serveraddrlen);
		
		if(l < 0)
			return -1;
		return l;
	}
	
	//Send to the server
	bool write(uint8_t* pld, int lth) {
		//TODO: MARSHALING
		
		lsp_packet p;
		p.id;
		p.sn;
		p.d = pld;
		
		int rc = sendto(sd, pld, lth, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
		
		if (rc < 0)
			return 0;
		else 
			return 1;
	}

};

