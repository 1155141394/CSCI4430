#include <arpa/inet.h>		// ntohs()
#include <stdio.h>		// printf(), perror()
#include <stdlib.h>		// atoi()
#include <string.h>		// strlen()
#include <sys/socket.h>		// socket(), connect(), send(), recv()
#include <unistd.h>		// close()
#include <netdb.h>
#include <netinet/in.h>
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
 
  int get_port_number(int sockfd) {
 	struct sockaddr_in addr;
	socklen_t length = sizeof(addr);
	if (getsockname(sockfd, (sockaddr *) &addr, &length) == -1) {
		perror("Error getting port of socket");
		return -1;
	}
	// Use ntohs to convert from network byte order to host byte order.
	return ntohs(addr.sin_port);
 }

 
int send_message(const char *hostname, int port, const char *message) {
	if (strlen(message) > MAX_MESSAGE_SIZE) {
		perror("Error: Message exceeds maximum length\n");
		return -1;
	}

	// (1) Create a socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// (2) Create a sockaddr_in to specify remote host and port
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	struct hostent *host = gethostbyname(hostname);
	if (host == nullptr) {
		fprintf(stderr, "%s: unknown host\n", hostname);
		return -1;
	}
	memcpy(&(addr.sin_addr), host->h_addr, host->h_length);
	addr.sin_port = htons(port);

	// (3) Connect to remote server
	if (connect(sockfd, (sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("Error connecting stream socket");
		return -1;
	}
	
	// (4) Send message to remote server
	// Call send() enough times to send all the data
	ssize_t message_len = strlen(message);
	ssize_t sent = 0;
	do {
		ssize_t n = send(sockfd, message + sent, message_len - sent, 0);
		if (n == -1) {
			perror("Error sending on stream socket");
			return -1;
		}
		sent += n;
	} while (sent < message_len);

	// (5) Close connection
	close(sockfd);

	return 0;
}

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

	// (1) Receive message from client.

	char msg[MAX_MESSAGE_SIZE + 1];
	memset(msg, 0, sizeof(msg));

	// Call recv() enough times to consume all the data the client sends.
	ssize_t recvd = 0;
	ssize_t rval;
	do {
		// Receive as many additional bytes as we can in one call to recv()
		// (while not exceeding MAX_MESSAGE_SIZE bytes in total).
		rval = recv(connectionfd, msg + recvd, MAX_MESSAGE_SIZE - recvd, 0);
		if (rval == -1) {
			perror("Error reading stream message");
			return -1;
		}
		recvd += rval;
	} while (rval > 0);  // recv() returns 0 when client closes

	// (2) Print out the message
	printf("Client %d says '%s'\n", connectionfd, msg);

	// (4) Close connection
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
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("Error opening stream socket");
		return -1;
	}

	// (2) Set the "reuse port" socket option
	int yesval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yesval, sizeof(yesval)) == -1) {
		perror("Error setting socket options");
		return -1;
	}

	// (3) Create a sockaddr_in struct for the proper port and bind() to it.
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port); 

	// (3b) Bind to the port.
	if (bind(sockfd, (sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("Error binding stream socket");
		return -1;
	}

	// (3c) Detect which port was chosen.
	port = get_port_number(sockfd);
	printf("Server listening on port %d...\n", port);

	// (4) Begin listening for incoming connections.
	listen(sockfd, queue_size);

	// (5) Serve incoming connections one by one forever.
	while (true) {
		int connectionfd = accept(sockfd, 0, 0);
		if (connectionfd == -1) {
			perror("Error accepting connection");
			return -1;
		}

		if (handle_connection(connectionfd) == -1) {
			return -1;
		}
	}
}

int main(int argc, const char **argv) {
	if (argc ==2){
		int port = atoi(argv[1]);
		if (run_server(port, 10) == -1) {
			return 1;
		}
		return 0;
	}
	else if (argc ==4 ){
		const char *hostname = argv[1];
		int port = atoi(argv[2]);
		const char *message = argv[3];
		printf("Sending message %s to %s:%d\n", message, hostname, port);
		if (send_message(hostname, port, message) == -1) {
			return 1;
		}
	
		return 0;
	}
	else{
		printf("Unknow command, try again");
		return 1;
	}
	
}

