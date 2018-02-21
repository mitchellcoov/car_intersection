// Keyboard_Control.cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <JHPWMPCA9685.h>
#include <iostream>
#include <omp.h>
#include <netdb.h> 


#define PWM_FULL_REVERSE 204	// Full right
#define PWM_NEUTRAL 307		// Center (ESC range from ??? to 339
#define PWM_FULL_FORWARD 509	// Full left

#define ESC_CHANNEL 0
#define STEERING_CHANNEL 1

int area[256];

/** area**/
//Prints an error message with the provided message and closes program
void error(const char *msg);

void area_retriever(int starting_portno);


/** intersection**/
void run_client(void);

void listener(void);


int main() {
/** area**/
     //Declare variables
     omp_set_nested(1);
     int portno = 13000;
     #pragma omp parallel num_threads(2)
     if(omp_get_thread_num() == 0)   {  
          area_retriever(portno);
     } else { 

/** motor control**/

    PCA9685 *pca9685 = new PCA9685() ;
   
    int err = pca9685->openPCA9685();
    if (err < 0){
     	printf("Error: %d", pca9685->error);
    }
    printf("PCA9685 Device Address: 0x%02X\n",pca9685->kI2CAddress) ;
    pca9685->setAllPWM(0,0);
    pca9685->reset();
    pca9685->setPWMFrequency(50);

    // init wheels to center and motor speed to 0

    int currentChannel = 0;
//    float currentPWM = PWM_NEUTRAL;
    float currentPWM = 342;		// debug start w/ speed
    float current_pwm_angle = PWM_NEUTRAL;
    sleep(5);
    pca9685->setPWM(ESC_CHANNEL,0,currentPWM);
//    pca9685->setPWM(ESC_CHANNEL,0,PWM_NEUTRAL);
    pca9685->setPWM(STEERING_CHANNEL,0,current_pwm_angle);

    printf("ESC Motor initialized\n");
    printf("Steering Motor initialized\n");	

    struct termios old_t, new_t;
    tcgetattr(STDIN_FILENO, &old_t);
    new_t = old_t;
    new_t.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t);

    while(pca9685->error >= 0){

	//char inp = std::cin.get();
	char inp = 'h';

	if(inp == 'i') {
	    printf("stop sign detected\n");
	    currentChannel = ESC_CHANNEL;
	    currentPWM = PWM_NEUTRAL;
	    pca9685->setPWM(currentChannel, 0, currentPWM);
	}

	if(area[0] > 4000) {
	    currentChannel = ESC_CHANNEL;
	    currentPWM = PWM_NEUTRAL;
	    pca9685->setPWM(currentChannel, 0, currentPWM);
	    #pragma omp parallel num_threads(3)
	    {
		switch(omp_get_thread_num()) {
		    case 1:
			printf("listening %i\n", omp_get_thread_num());
			listener();
			currentChannel = ESC_CHANNEL;
	    		currentPWM = 344;
			pca9685->setPWM(currentChannel, 0, currentPWM);
		    case 2:
			printf("clienting %i\n", omp_get_thread_num());
			run_client();
		    default:
			printf("defaulting %i\n", omp_get_thread_num());
			break;
		}
	    }
	}

	if(inp == 's') { 
	   printf("Decreasing speed\n");
	   currentChannel = ESC_CHANNEL;
	   if(currentPWM > 337) {
		currentPWM--;
		printf("Current speed = %f\n", currentPWM);
	        pca9685->setPWM(currentChannel, 0, currentPWM);
	   }
	}

	if(inp == 'w') {
	   printf("Increasing speed\n");
	   currentChannel = ESC_CHANNEL;

	   // increase to 340 instead if between ??? and 338
	   if(currentPWM <= 338) {
	       currentPWM = 340;
	   } else {
	       currentPWM++;
	   }
	   printf("Current speed = %f\n", currentPWM);
	   pca9685->setPWM(currentChannel, 0, currentPWM);
	}

	if(inp == 'a') {
	   printf("turning left\n");
	   currentChannel = STEERING_CHANNEL;
	   current_pwm_angle+=20;
	   pca9685->setPWM(currentChannel, 0, current_pwm_angle);
	}

	if(inp == 'd') {
	   printf("turning right\n");
	   currentChannel = STEERING_CHANNEL;
	   current_pwm_angle-=20;
	   pca9685->setPWM(currentChannel, 0, current_pwm_angle);
	}
	    if(inp == 'e') {
	    printf("Stopping car\n");
	    pca9685->setPWM(ESC_CHANNEL,0,PWM_NEUTRAL);
    	    pca9685->setPWM(STEERING_CHANNEL,0,PWM_NEUTRAL);
   	    pca9685->closePCA9685();

    	    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
	    
	}
    }
    pca9685->setPWM(ESC_CHANNEL,0,PWM_NEUTRAL);
    pca9685->setPWM(STEERING_CHANNEL,0,PWM_NEUTRAL);
    pca9685->closePCA9685();

    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
  }
}

/**server**/
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void area_retriever(int starting_portno) {
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     int buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = starting_portno;
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

     fprintf(stdout,"Ready to receive on port %i\n", portno);
     char bufferend[] = "close socket\n";
     while(1){
          listen(sockfd,5);
          clilen = sizeof(cli_addr);
          newsockfd = accept(sockfd, 
                      (struct sockaddr *) &cli_addr, 
                      &clilen);
          if (newsockfd < 0) 
               error("ERROR on accept");
//          bzero(buffer,256);
          n = read(newsockfd,buffer,255);
	  area[0] = buffer[0];
          if (n < 0) error("ERROR reading from socket");
        //  printf("Here is the message on port %i: %i \n", portno, buffer[0]);
	  printf("Here is the buffer: %i, and here is the area: %i \n", buffer[0], area[0]);
          n = write(newsockfd,"I got your message",18);
          if (n < 0) error("ERROR writing to socket");
     }

     close(newsockfd);
     close(sockfd);
}

void run_client(void) {
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	char host[14];
	strcpy(host,"192.168.0.101");
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(host);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
	serv_addr.sin_port = htons(12345);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	bzero(buffer, 256);
	strcpy(buffer,"here!");
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

void listener(void) {
	int sockfd, newsockfd, n, stop;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int portno = 21345;

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

	fprintf(stdout, "Listening on port %i\n", portno);
	char bufferend[] = "close socket\n";
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
	printf("Here is the message: %s \n", buffer);
	n = write(newsockfd, "Acknowledge", 50);
	if (n < 0) error("ERROR writing to socket");

	close(newsockfd);
	close(sockfd);
}


