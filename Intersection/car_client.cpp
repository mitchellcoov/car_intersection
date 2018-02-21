#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <omp.h>
#include <fstream>

void error(const char *msg);

void run_client(char* host_name, int portno);

int main(int argc, char *argv[])
{

	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}

	int portno = atoi(argv[2]);
	run_client(argv[1], portno);

	return 0;
}

void run_client(char* host_name, int portno) {
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(host_name);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	bzero(buffer, 256);
	printf("type something");
	fgets(buffer, 255, stdin);
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

void error(const char *msg)
{
	perror(msg);
	exit(0);
}
