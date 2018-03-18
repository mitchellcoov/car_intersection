// Keyboard_Control.cpp
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <string>
#include <sstream>
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
#define PWM_FULL_FORWARD 410	// Full left OR 509

#define ESC_CHANNEL 0
#define STEERING_CHANNEL 1

int area[256];

bool recentStop = false;

int listeningPorts[2];

const char *stringListeningPorts;



int randPort(float irand);


/** area**/
//Prints an error message with the provided message and closes program
void error(const char *msg);

void area_retriever(int starting_portno);


/** intersection**/
void run_client(void);

void listener(int portno);


int main() {

    srand(time(NULL));

    std::stringstream ss0;
    std::stringstream ss1;
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
	// TODO Call laneFollower here instead of the following hardcoded values
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
				listeningPorts[0] = randPort((float)rand());
				listeningPorts[1] = randPort((float)rand());
				ss0 << listeningPorts[0];
	
				ss1 << listeningPorts[1];
		
				std::string stringPorts = ss0.str() + ":" + ss1.str();
				std::cout <<"string ports are " << stringPorts << '\n';
				stringListeningPorts = stringPorts.c_str();
				printf("char ports are %s", stringListeningPorts);

				ss0.clear();
				ss0.str("");
				ss1.clear();
				ss1.str("");
			#pragma omp parallel num_threads(3)
			{

			printf("number of threads: %i\n", omp_get_num_threads());
			switch(omp_get_thread_num()) {
				case 1:
				printf("listening %i\n", omp_get_thread_num());
				recentStop = true;

				// Listen for the go ahead
				listener(listeningPorts[0]);
				// TODO Call laneFollower here instead of the following hardcoded values
				currentChannel = ESC_CHANNEL;
					currentPWM = 342;
				pca9685->setPWM(currentChannel, 0, currentPWM);

				// Listen for cleared intersection
				listener(listeningPorts[1]);
				recentStop = false;
				break;

				case 2:
				printf("clienting %i\n", omp_get_thread_num());
				run_client();
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
              error("ERROR on binding area_retriever");

     fprintf(stdout,"Ready to receive on port %i\n", portno);
     char bufferend[] = "close socket\n";
     while(1){
	  if (!recentStop) {
              listen(sockfd,5);
              clilen = sizeof(cli_addr);
              newsockfd = accept(sockfd, 
                      (struct sockaddr *) &cli_addr, 
                      &clilen);
              if (newsockfd < 0) 
                  error("ERROR on accept");
//               bzero(buffer,256);
              n = read(newsockfd,buffer,255);
              area[0] = buffer[0];
              if (n < 0) error("ERROR reading from socket");
        //  printf("Here is the message on port %i: %i \n", portno, buffer[0]);
	  printf("Here is the buffer: %i, and here is the area: %i \n", buffer[0], area[0]);
          n = write(newsockfd,"I got your message",18);
          if (n < 0) error("ERROR writing to socket");
	  }

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
	
	printf("char ports are %s", stringListeningPorts);
	strcpy(buffer, stringListeningPorts);
	printf("buffer is: %s", buffer); 
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

void listener(int portno) {


	int sockfd, newsockfd, n, stop;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0)
		error("ERROR on binding listener");

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
	if (recentStop) {
	    n = write(newsockfd, "Acknowledge GO", 50);
	} else {
	    n = write(newsockfd, "Acknowledge CLEAR", 50);
	}
	if (n < 0) error("ERROR writing to socket");



	close(newsockfd);
	close(sockfd);
}

int randPort(float irand) {
    float frand = (irand / RAND_MAX)*(100) + 21300;
    return (int)frand;
}


// notes for implementation:
// Maybe do some sort of offset to make negative/positive values from the
// difference of car and lane midpoint easier to work with
// Maybe only make adjustments when midPointDiff is greater than some value?

// Skeleton for lane follower
// Max left turn value  = PWM_FULL_FORWARD
// Max right turn value = PWM_FULL_REVERSE
// Straight angle value	= PWM_NEUTRAL
// Max speed value		= unknown
// Stop speed value		= PWM_NEUTRAL
void laneFollower(/**arguments from lane detection**/) {	// TODO forward declaration

	// offset pwm signal values for easier caculations
	// Neutral value becomes 0
	// Full right becomes -103
	// Full left becomes 103
	int offset = PWM_NEUTRAL;
	int OFFSET_FULL_FORWARD = PWM_FULL_FORWARD - offset;
	int OFFSET_FULL_REVERSE = PWM_FULL_REVERSE - offset;
	int OFFSET_FULL_NEUTRAL = PWM_FULL_NEUTRAL - offset;

	//calculate midPointDiff from function parameters
	float midPointDiff = 0;	// TODO calc from function args

	// TODO figure out constants empirically
	// angle constants to properly tune speed/angle changes
	// careful! these are floats and pwm needs to be an int, remember to cast
	float speedConstant = 1.f;
	float angleConstant = 1.f;

	// motor controls for when the car is not in center of the lane
	if (fabs(midPointDiff)) > 0 {

		// Skeleton for changing speed
		// The greater the difference between car and lane midpoint
		// The lower the currentPWM should be set
		currentChannel = ESC_CHANNEL;
		currentPWM = OFFSET_FULL_FORWARD * (1/midPointDiff) * speedConstant;
		currentPWM += offset;	// remove offset
		pca9685->setPWM(currentChannel, 0, currentPWM);

		// Skeleton for changing direction
		// The greater the difference between car and lane midpoint
		// the greater the current_pwm_angle should be set
		currentChannel = STEERING_CHANNEL;
		current_pwm_angle = OFFSET_FULL_FORWARD * midPointDiff * angleConstant;
		current_pwm_angle += offset;	// remove offset
		pca9685->setPWM(currentChannel, 0, current_pwm_angle);
	}
}




