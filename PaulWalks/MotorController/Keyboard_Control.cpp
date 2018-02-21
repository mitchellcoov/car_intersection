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


#define PWM_FULL_REVERSE 204	// Full right
#define PWM_NEUTRAL 307		// Center (ESC range from ??? to 339
#define PWM_FULL_FORWARD 509	// Full left

#define ESC_CHANNEL 0
#define STEERING_CHANNEL 1

int area[256];

/** server**/
//Prints an error message with the provided message and closes program
void error(const char *msg);

void run_server(int starting_portno);
/** server**/

int main() {
/** server**/
     //Declare variables

     int portno = 13000;
     #pragma omp parallel num_threads(2)
     if(omp_get_thread_num() == 0)   {  
          run_server(portno);
     } else { 
/** server**/

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
    float currentPWM = 338;		// debug start w/ speed
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

	char inp = std::cin.get();
	//char inp = 'h';
	printf("area is: %i \n", area[0]);
	if(inp == 'i') {
	    printf("Stop sign detected\n");
	    currentChannel = ESC_CHANNEL;
	    currentPWM = PWM_NEUTRAL;
	    pca9685->setPWM(currentChannel, 0, currentPWM);
	}
	if(area[0] > 4000) {
	    currentChannel = ESC_CHANNEL;
	    currentPWM = PWM_NEUTRAL;
	    pca9685->setPWM(currentChannel, 0, currentPWM);
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

void run_server(int starting_portno) {
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

