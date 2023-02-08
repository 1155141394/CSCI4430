#include <arpa/inet.h>		// htons()
#include <stdio.h>		// printf(), perror()
#include <stdlib.h>		// atoi()
#include <sys/socket.h>		// socket(), bind(), listen(), accept(), send(), recv()
#include <unistd.h>		// close()
#include <time.h>
#include "helpers.h"		// make_server_sockaddr(), get_port_number()

static const size_t MAX_MESSAGE_SIZE = 256;

/**
 * Receives a string message from the client and prints it to stdout.
 *
 * Parameters:
 * 		connectionfd: 	File descriptor for a socket connection
 * 				(e.g. the one returned by accept())
 * Returns:
 *		0 on success, -1 on failure.
 */
int handle_connection(int connectionfd) {

	printf("New connection %d\n", connectionfd);
	int recvbytes;
	char buf[MAX_MESSAGE_SIZE];//传输的数据
	// (1) Receive message from client.
	if((recvbytes = recv(connectionfd,buf,MAX_MESSAGE_SIZE,0)) == -1) {//接收客户端的请求
        perror("recv");
       return -1;
    }

	// (2) Print out the message
	printf("received a connection : %s\n",buf);
	// (3) Close connection
	close(connectionfd);
	return 0;
}

/**
 * Endlessly runs a server that listens for connections and serves
 * them _synchronously_.
 *
 * Parameters:
 *		port: 		The port on which to listen for incoming connections.
 *		queue_size: 	Size of the listen() queue
 * Returns:
 *		-1 on failure, does not return on success.
 */
int run_server(int port, int queue_size) {

	// (1) Create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// (2) Set the "reuse port" socket option

	// (3) Create a sockaddr_in struct for the proper port and bind() to it.
	struct sockaddr_in addr;
	if (make_server_sockaddr(&addr, port) == -1) {
		return -1;
	}

	// (3b) Bind to the port.
	memset(&addr, 0, sizeof(addr)); 
	addr.sin_family = AF_INET; 
	addr.sin_addr.s_addr = INADDR_ANY; 
	addr.sin_port = htons(port);
	bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));

	// (3c) Detect which port was chosen.
	port = get_port_number(sockfd);
	printf("Server listening on port %d...\n", port);

	// (4) Begin listening for incoming connections.
	listen(sockfd, 10);
	// (5) Serve incoming connections one by one forever.
	socklen_t addr_len = sizeof(addr); 
	int conn = accept(sockfd, (struct sockaddr *) &addr, &addr_len);
	printf("客户端%s连接成功",inet_ntoa(addr.sin_addr));     
	handle_connection(conn);  
	return 0;

}

int main(int argc, const char **argv) {
	// Parse command line arguments
	time_t t1,t2;
	clock_t s,e;
	s = clock();
	t1 = time(0);
	for(int i=0;i<2000000000;i++){

	}
	t2 = time(0);
	e = clock();
	printf("%ld\n",t2-t1);
	printf("%f",(double)(e-s)/CLOCKS_PER_SEC);

	return 0;
}
