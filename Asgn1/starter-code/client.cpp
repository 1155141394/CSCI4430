#include <arpa/inet.h>		// ntohs()
#include <stdio.h>		// printf(), perror()
#include <stdlib.h>		// atoi()
#include <string.h>		// strlen()
#include <sys/socket.h>		// socket(), connect(), send(), recv()
#include <unistd.h>		// close()

#include "helpers.h"		// make_client_sockaddr()

static const int MAX_MESSAGE_SIZE = 256;

/**
 * Sends a string message to the server.
 *
 * Parameters:
 *		hostname: 	Remote hostname of the server.
 *		port: 		Remote port of the server.
 * 		message: 	The message to send, as a C-string.
 * Returns:
 *		0 on success, -1 on failure.
 */
int send_message(const char *hostname, int port, const char *message) {
	
	if (strlen(message) > MAX_MESSAGE_SIZE) {
		perror("Error: Message exceeds maximum length\n");
		return -1;
	}
	
	// (1) Create a socket
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	struct hostent *server = gethostbyname(hostname); 

	// (2) Create a sockaddr_in to specify remote host and port
	struct sockaddr_in addr;
	if (make_client_sockaddr(&addr, hostname, port) == -1) {
		return -1;
	}
	
	// (3) Connect to remote server
	memset(&addr, 0, sizeof(addr)); 
	addr.sin_family = AF_INET; 
	//addr.sin_addr.s_addr = * (unsigned long *) server->h_addr_list[0]; 
	addr.sin_port = htons(port);

	int fail = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));//开始连接
    if (fail){
        printf("与服务端连接失败！程序将退出...");
        return -1;
    }
	// (4) Send message to remote server
	
	

	send(sockfd, message, MAX_MESSAGE_SIZE, MSG_NOSIGNAL);

	// (5) Close connection
	close(sockfd);

	return 0;
}

int main(int argc, const char **argv) {
	// Parse command line arguments
	if (argc != 4) {
		printf("Usage: ./client hostname port_num message\n");
		return 1;
	}
	
	const char *hostname = argv[1];
	int port = atoi(argv[2]);
	const char *message = argv[3];
	
	//printf("Sending message %s to %s:%d\n", message, hostname, port);
	printf("%s\n",hostname);
	if (send_message(hostname, port, message) == -1) {
		return 1;
	}

	return 0;
}
