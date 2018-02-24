/* A simple server in the internet domain using TCP
   The port number is passed as an argument
   Reference: http://www.linuxhowtos.org/C_C++/socket.htm*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <omp.h>
#include <queue>
#include <string>

//Prints an error message with the provided message and closes program
void error(const char *msg);

//listens for incoming cars, adds them to the queue
void listener(int starting_portno);

void handler(void);

void sendMessage(sockaddr_in cli_addr, const char message[], int portno);

std::queue<sockaddr_in> q;

int clientPorts[2];

int main(int argc, char *argv[])
{
	//Declare variables
	int thread_count = 2;

	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	int portno = atoi(argv[1]);
	#pragma omp parallel num_threads(thread_count) 
	{
		switch (omp_get_thread_num()) {
			case 0:
				listener(portno);
			case 1:
				handler();
			default:
				break;
		}
	}

	return 0;
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void listener(int portno) {
	int sockfd, newsockfd;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n, stop;
	int thread_rank = omp_get_thread_num();

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	fprintf(stdout, "Thread %i is ready to receive on port %i\n", thread_rank, portno);
	char bufferend[] = "close socket\n";
	while (1) {
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd,
			(struct sockaddr *) &cli_addr,
			&clilen);
		if (newsockfd < 0)
			error("ERROR on accept");
		bzero(buffer, 256);
		n = read(newsockfd, buffer, 255);
		if (n < 0) error("ERROR reading from socket");
		printf("Comparing message\n");
		stop = memcmp(buffer, bufferend, sizeof(bufferend));
		if (stop == 0) break;
		printf("Here is the message: %s \n", buffer);
		const char s[2] = ":";
		char *messagePorts = strtok(buffer,s);
		printf("message ports 0: %s\n", messagePorts);
		clientPorts[0] = std::stoi(messagePorts);
		messagePorts = strtok(NULL,s);
		clientPorts[1] = std::stoi(messagePorts);
		//add to queue
		q.push(cli_addr);
		n = write(newsockfd, "I got your message, you were added to the queue", 50);
		if (n < 0) error("ERROR writing to socket");
	}

	close(newsockfd);
	close(sockfd);
}

void handler(void) {
	while (1) {
		if (!q.empty()) {
			printf("queue isnt empty\n");
			sleep(10);
			printf("sending message to %d\n", clientPorts[0]);
			sendMessage(q.front(), "proceed", clientPorts[0]);
			sleep(10);
			sendMessage(q.front(), "cleared", clientPorts[1]);
			q.pop();
		}
	}
	
}

void sendMessage(sockaddr_in cli_addr, const char message[], int portno) {
	int sockfd, n;
	struct sockaddr_in serv_addr;
	char buffer[256];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)&cli_addr.sin_addr.s_addr,
		(char *)&serv_addr.sin_addr.s_addr,
		sizeof(cli_addr.sin_addr.s_addr));
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	bzero(buffer, 256);
	printf("replying with message: %s...\n",message);
	strcpy(buffer,message);
	n = write(sockfd, buffer, strlen(buffer));
	if (n < 0)
		error("ERROR writing to socket");
	bzero(buffer, 256);
	n = read(sockfd, buffer, 255);
	if (n < 0)
		error("ERROR reading from socket");
	printf("%s\n", buffer);
	close(sockfd);
}







