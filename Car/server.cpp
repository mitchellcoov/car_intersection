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

//Prints an error message with the provided message and closes program
void error(const char *msg);

void run_server(int starting_portno);

int main(int argc, char *argv[])
{
     //Declare variables
     int thread_count = 3;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     int portno = atoi(argv[1]);
     #pragma omp parallel num_threads(thread_count)
     run_server(portno);
   
     return 0; 
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void run_server(int starting_portno) {
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n, stop;
     int thread_rank = omp_get_thread_num();

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = starting_portno + thread_rank;
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

     fprintf(stdout,"Thread %i is ready to receive on port %i\n", thread_rank, portno);
     char bufferend[] = "close socket\n";
     while(1){
          listen(sockfd,5);
          clilen = sizeof(cli_addr);
          newsockfd = accept(sockfd, 
                      (struct sockaddr *) &cli_addr, 
                      &clilen);
          if (newsockfd < 0) 
               error("ERROR on accept");
          bzero(buffer,256);
          n = read(newsockfd,buffer,255);
          if (n < 0) error("ERROR reading from socket");
          printf("Comparing message\n");
          stop=memcmp (buffer, bufferend, sizeof(bufferend));
          printf("here is stop %d \n", stop);
          if (stop==0) break; 
          printf("Here is the message on port %i (presented by thread %i): %s \n", portno, thread_rank, buffer);
          n = write(newsockfd,"I got your message",18);
          if (n < 0) error("ERROR writing to socket");
     }

     close(newsockfd);
     close(sockfd);
}



