#include <arpa/inet.h>		// htons()
#include <stdio.h>		// printf(), perror()
#include <stdlib.h>		// atoi()
#include <sys/socket.h>		// socket(), bind(), listen(), accept(), send(), recv()
#include <unistd.h>		// close()
#include <time.h>
#include "helpers.h"		// make_server_sockaddr(), get_port_number()

static const size_t MAX_MESSAGE_SIZE = 1000;

int handle_connection(int connectionfd) {
	printf("New connection %d\n", connectionfd);
	int recvbytes;
	char buf[MAX_MESSAGE_SIZE];//传输的数据

	// (1) Receive message from client.
    clock_t start_t, end_t;
	int received = 0;
    start_t = clock();
    while(1){
        memset(buf,0,sizeof(buf));
		if((recvbytes = recv(connectionfd,buf,sizeof(buf),0)) == -1) {//接收客户端的请求
            perror("recv");
            return -1;
        }
		int len = strlen(buf);
        if(buf[len-1]=='e'){
            printf("Connect finished\n");
			send(connectionfd,"f",1,0);
            break;
        }
        // (2) Print out the message
        received += recvbytes;
	    //printf("received a connection : %s\n",buf);
       
	}
    end_t = clock();
	received = (int)(received/1000);
    double total_t = (double)(end_t - start_t)/CLOCKS_PER_SEC;
    double rate = received*8/(1000*total_t);
    printf("Received=%d KB\n",received);
    printf("Rate=%.3f Mbps\n",rate);
	
	// (3) Close connection
	if((recvbytes = recv(connectionfd,buf,sizeof(buf),0))==0){
		close(connectionfd);
	}
	return 0;
}

int run_server(int port, int queue_size) {

	// (1) Create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// (2) Set the "reuse port" socket option
	int yesval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yesval, sizeof(yesval)) == -1) {
		perror("Error setting socket options");
		return -1;
	}
	// (3) Create a sockaddr_in struct for the proper port and bind() to it.
	struct sockaddr_in addr;
	if (make_server_sockaddr(&addr, port) == -1) {
		return -1;
	}

	// (3b) Bind to the port.
	// memset(&addr, 0, sizeof(addr)); 
	// addr.sin_family = AF_INET; 
	// addr.sin_addr.s_addr = INADDR_ANY; 
	// addr.sin_port = htons(port);
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
	socklen_t addr_len = sizeof(addr); 
	int conn = accept(sockfd, (struct sockaddr *) &addr, &addr_len);
	printf("客户端%s连接成功",inet_ntoa(addr.sin_addr));     
	handle_connection(conn);  
	close(sockfd);
	return 0;

}

int send_message(const char *hostname, int port, int interval) {
	char message[MAX_MESSAGE_SIZE] = {0}; 
	// (1) Create a socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	struct hostent *server = gethostbyname(hostname); 

	// (2) Create a sockaddr_in to specify remote host and port
	struct sockaddr_in addr;
	if (make_client_sockaddr(&addr, hostname, port) == -1) {
		return -1;
	}
	
	// (3) Connect to remote server
	// memset(&addr, 0, sizeof(addr)); 
	// addr.sin_family = AF_INET; 
	// addr.sin_addr.s_addr = * (unsigned long *) server->h_addr_list[0]; 
	// addr.sin_port = htons(port);

	int fail = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));//开始连接
    if (fail){
        printf("与服务端连接失败！程序将退出...");
        return -1;
    }
	// (4) Send message to remote server
	clock_t start_t, end_t, middle_t;
	start_t = clock();
	int sent = 0; 
	int sendbytes;
	while(1){
		middle_t = clock();
		if((double)(middle_t - start_t)/CLOCKS_PER_SEC>=interval){
			break;
		}
		if((sendbytes=send(sockfd, message, MAX_MESSAGE_SIZE, 0))==-1){
			return -1;
		}
		sent += sendbytes;
	}
	sent = (int)(sent/1000);
	send(sockfd,"e",1,0);
	int recvbytes = recv(sockfd,message,sizeof(message),0);
	int len = strlen(message);
	if(message[len-1]=='f'){
		end_t = clock();
	}
	double total_t = (double)(end_t - start_t)/CLOCKS_PER_SEC;
    double rate = sent*8/(1000*total_t);
    printf("Sent=%d KB\n",sent);
    printf("Rate=%.3f Mbps\n",rate);
	// (5) Close connection
	close(sockfd);

	return 0;
}




int main(int argc, const char **argv) {
	// Parse command line arguments
	
	const char *def = argv[1];
    if(strcmp(def,"-s") == 0){
        if(argc != 4){
            printf("Error: missing or extra arguments");
			return 1;
        }
        int port = atoi(argv[3]);
        
        if(port<1024 || port>65535){
            printf("Error: port number must be in the range of [1024, 65535]");
			return 1;
        }
        if (run_server(port, 10) == -1) {
		    return 1;
	    }
	    return 0;

    }else{
		if(argc != 8){
			printf("Error: missing or extra arguments\n");
		}
		const char *hostname = argv[3];
		int port = atoi(argv[5]);
		int time = atoi(argv[7]);
		if(port<1024 || port>65535){
            printf("Error: port number must be in the range of [1024, 65535]\n");
			return 1;
        }
		if(time<0){
			printf("Error: time argument must be greater than 0\n");
			return 1;
		}
		if (send_message(hostname, port, time) == -1) {
		return 1;
	}

	}

	return 0;
}