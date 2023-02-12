#include <arpa/inet.h>		// htons()
#include <stdio.h>		// printf(), perror()
#include <stdlib.h>		// atoi()
#include <sys/socket.h>		// socket(), bind(), listen(), accept(), send(), recv()
#include <unistd.h>		// close()
#include <time.h>
#include "helpers.h"		// make_server_sockaddr(), get_port_number()

static const size_t MAX_MESSAGE_SIZE = 1000;

int handle_connection(int connectionfd) {
	//printf("New connection %d\n", connectionfd);
	int recvbytes;
	char buf[10000];//传输的数据

	// (1) Receive message from client.
    time_t start_t, end_t;
	int received = 0;
    time(&start_t);
    while(1){
		if((recvbytes = recv(connectionfd,&buf,1000,MSG_NOSIGNAL)) == -1) {//接收客户端的请求
            perror("recv");
            return -1;
        }
		received += recvbytes;
        if(buf[recvbytes-1]=='1'){
            //printf("Connect finished\n");
			char finish[2] = "0";
			send(connectionfd,&finish,strlen(finish),MSG_NOSIGNAL);
            break;
        }
        
	    //printf("received a connection : %s\n",buf);
       
	}
    time(&end_t);
	received = received/1000;
    double total_t = difftime(end_t,start_t);
    double rate = received*8/(1000*total_t);
    printf("Received=%d KB, Rate=%.3f Mbps\n",received,rate);
	
	// (3) Close connection
	if((recvbytes = recv(connectionfd,buf,sizeof(buf),MSG_NOSIGNAL))==0){
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
	//memset(&addr, 0, sizeof(addr)); 
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
	handle_connection(conn);  
	shutdown(sockfd,1);
	close(sockfd);
	return 0;

}

int send_message(const char *hostname, int port, int interval) {
	char message[MAX_MESSAGE_SIZE] = {0}; 
	memset(message, '0', 1000);
	// (1) Create a socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	struct hostent *server = gethostbyname(hostname); 

	// (2) Create a sockaddr_in to specify remote host and port
	struct sockaddr_in addr;
	if (make_client_sockaddr(&addr, hostname, port) == -1) {
		return -1;
	}
	
	// (3) Connect to remote server
	//memset(&addr, 0, sizeof(addr)); 
	// addr.sin_family = AF_INET; 
	// addr.sin_addr.s_addr = * (unsigned long *) server->h_addr_list[0]; 
	// addr.sin_port = htons(port);

	int fail = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));//开始连接
    if (fail){
        printf("与服务端连接失败！程序将退出...");
        return -1;
    }
	// (4) Send message to remote server
	time_t start_t, end_t, middle_t;
	time(&start_t);
	int sent = 0; 
	int sendbytes;
	while(1){
		time(&middle_t);
		if(difftime(middle_t,start_t)>=(double)interval){
			break;
		}
		if((sendbytes=send(sockfd, &message, strlen(message), MSG_NOSIGNAL))==-1){
			return -1;
		}
		sent += sendbytes;
	}
	sent = sent/1000;
	char finish[2]="1";
	send(sockfd,finish,strlen(finish),MSG_NOSIGNAL);
	time(&end_t);
	double total_t = difftime(end_t, start_t);
    double rate = sent*8/(1000*total_t);
    printf("Sent=%d KB, Rate=%.3f Mbps\n",sent,rate);
	// (5) Close connection
	shutdown(sockfd,1);
	close(sockfd);
	return 0;
}




int main(int argc, const char **argv) {
	// Parse command line arguments
	if(argc != 4 && argc != 8){
		printf("Error: missing or extra arguments");
			return 1;
	}
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

    }else if(strcmp(def,"-c") == 0){
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
		if(time<=0){
			printf("Error: time argument must be greater than 0\n");
			return 1;
		}
		if (send_message(hostname, port, time) == -1) {
		return 1;
		}

	}else{
		printf("Error: missing or extra arguments\n");
	}

	return 0;
}