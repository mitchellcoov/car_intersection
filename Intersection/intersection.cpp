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
#include <sstream>
#include <iostream>
#include <vector>


int carCoords[4];


//Prints an error message with the provided message and closes program
void error(const char *msg);

//listens for incoming cars, adds them to the queue
void listener(int starting_portno);

void handler(void);

void sendMessage(sockaddr_in cli_addr, const char message[], int portno);

void pointRetriever(int starting_portno);

std::queue<sockaddr_in> q;

int clientPorts[2];

class Point
{
    private:
        int x, y;
    public:
        Point() : x(0), y(0) {}
        Point(int x, int y) : x(x), y(y) {}
	int getX() 
	{
		return x;
	}

	int getY() 
	{
		return y;
	}
		
};

class Line
{
    private:
        Point p1;
        Point p2;
		double slope;
		double yIntersect;
    public:

		Line() : p1(Point()), p2(Point()) {}
		
        Line(Point & p1, Point & p2 ) : p1(p1), p2(p2) {
			slope = ((double)p2.getY() - (double)p1.getY())/((double)p2.getX() - (double)p1.getX());
			yIntersect = p1.getY() - slope*p1.getX();
		}

        void setPoints( const Point & ap1, const Point & ap2)
        {
            p1 = ap1;
            p2 = ap2;
        }

		Point getPoint1() {
			return p1;
		}
		Point getPoint2() {
			return p2;
		}
		
		double getSlope() {
			return slope;
		}
		double getYIntersect() {
			return yIntersect;
		}
		bool isIntersectLegal(Point p) {
			return (std::min(p1.getX(),p2.getX()) <= p.getX() && p.getX() <= std::max(p1.getX(),p2.getX()));
		/**			
			} else {
				return (std::min(p1.getY(),p2.getY()) <= p.getY() && p.getY() <= std::max(p1.getY(),p2.getY ()));
			}		
		**/
				
		}

};

bool doLinesIntersect(Line l1, Line l2);

Line extendPoint(Point p);

void initIntersection();

bool inIntersection(Line lines[], Line boundLines[]);

Point intPoints[4];
Line intLines[4];
Point blPoints[4];
Line blLines[4];


int main(int argc, char *argv[])
{
	//Declare variables
	int thread_count = 3;
	initIntersection();

	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	int portno = atoi(argv[1]);
	int portnopoint = 13000;

	#pragma omp parallel num_threads(thread_count) 
	{
		switch (omp_get_thread_num()) {
			case 0:
				listener(portno);
			case 1:
				handler();
			case 2:
				pointRetriever(portnopoint);
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
	/**
	bool isCarPointIn[4] = {false};
	int countTrues = 0;
	bool lIntersections[4] = {false};
	**/

	while (1) {
	
		Point points[4];
		points[0] = Point(carCoords[0], carCoords[1]);
		points[1] = Point(carCoords[0] + carCoords[2], carCoords[1]);
		points[2] = Point(carCoords[0], carCoords[1] + carCoords[3]);
		points[3] = Point(carCoords[0] + carCoords[2], carCoords[1] + carCoords[3]);

		Line lines[4];
		lines[0] = extendPoint(points[0]);
		lines[1] = extendPoint(points[1]);
		lines[2] = extendPoint(points[2]);
		lines[3] = extendPoint(points[3]);

		inIntersection(lines, intLines);
	
/**		
		for (int i = 0; i < 4; i++) {
			
			for (int j = 0; j < 4; j++) {
				lIntersections[j] = doLinesIntersect(lines[i], intLines[j]);
			}
			int countIntersects = 0;
			for (int j = 0; j < 4; j++) {
				if (lIntersections[j]) { 
					countIntersects++;
					lIntersections[j] = false;
				}
			}
			if (countIntersects == 1) isCarPointIn[i] = true;
			//printf("count intersects %d\n", countIntersects);
		}
		
		countTrues = 0;
		for (int i = 0; i < 4; i++) {
			if (isCarPointIn[i]) {
				countTrues++;
				isCarPointIn[i] = false;
			}
		}

		printf("count trues %d\n", countTrues);
		if (countTrues >= 2) printf("car in intersection\n");
		

		if (!q.empty()) {
			printf("queue isnt empty\n");
			
			//call py function and wait til clear
			if (

			printf("sending message to %d\n", clientPorts[0]);
			sendMessage(q.front(), "proceed", clientPorts[0]);
			
			//call py function and wait til clear

			sendMessage(q.front(), "cleared", clientPorts[1]);
			q.pop();
		}
**/
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

void pointRetriever(int starting_portno) {

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
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
        error("ERROR on binding point_retriever");

    fprintf(stdout,"Ready to receive on port %i\n", portno);
    char bufferend[] = "close socket\n";
    while(1) {
    	listen(sockfd,5);
			clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, 
            		(struct sockaddr *) &cli_addr, 
                    &clilen);
            if (newsockfd < 0) 
            	error("ERROR on accept");
            bzero(buffer,256);
            n = read(newsockfd,buffer,255);

            std::string str(buffer);
			std::vector<int> vect;
    		std::stringstream ss(str);

    		int i;

    		while (ss >> i)
    		{
    		    vect.push_back(i);

    		    if (ss.peek() == ',')
    	        ss.ignore();
    		}
    		for (int k = 0; k < 4; k++)    carCoords[k] = vect.at(k);
	      
           	if (n < 0) error("ERROR reading from socket");
     
         	n = write(newsockfd,"I got your message",18);
          	if (n < 0) error("ERROR writing to socket");

     }

     close(newsockfd);
     close(sockfd);
}

bool doLinesIntersect(Line l1, Line l2) {

	int x1,x2,x3,x4;
	int y1,y2,y3,y4;
	x1 = l1.getPoint1().getX();
	x2 = l1.getPoint2().getX();
	x3 = l2.getPoint1().getX();
	x4 = l2.getPoint2().getX();
	y1 = l1.getPoint1().getY();
	y2 = l1.getPoint2().getY();
	y3 = l2.getPoint1().getY();
	y4 = l2.getPoint2().getY();
	
	int noma = (y3-y4)*(x1-x3)+(x4-x3)*(y1-y3);
	int denoma = (x4-x3)*(y1-y2)-(x1-x2)*(y4-y3);
	int nomb = (y1-y2)*(x1-x3)+(x2-x1)*(y1-y3);
	int denomb = (x4-x3)*(y1-y2)-(x1-x2)*(y4-y3);
	double ta = (double)noma/denoma;
	double tb = (double)nomb/denomb;

	if (ta <= 1 && ta >= 0 && tb <= 1 && tb >= 0) {
		//printf("point 1 %d,%d %d,%d point 2 %d,%d %d,%d here is ta %f here is tb %f\n", x1,x2,x3,x4,y1,y2,y3,y4,ta,tb);
		return true;
	}
	return false;

	/**
	double x = (l2.getYIntersect() - l1.getYIntersect())/(l1.getSlope() - l2.getSlope());
	double y = (l1.getYIntersect()*l1.getSlope() - l2.getYIntersect()*l2.getSlope())/(l1.getSlope() - l2.getSlope());
	
	Point p = Point(x,y);
	if (x < l1.getPoint1().getX()) return false;
	return l2.isIntersectLegal(p);
	**/
	
}

Line extendPoint(Point p) {
	Point pex = Point(p.getX()+1000,p.getY());
	//printf("pex x is %d pex y is %d\n",pex.getX(), pex.getY());
	return Line(p, pex);
}

void initIntersection() {
	intPoints[0] = Point(680, 240);
	intPoints[1] = Point(357, 66);
	intPoints[2] = Point(40, 290);
	intPoints[3] = Point(340, 450);
	intLines[0] = Line(intPoints[0],intPoints[1]);
	intLines[1] = Line(intPoints[0],intPoints[2]);
	intLines[2] = Line(intPoints[1],intPoints[3]);
	intLines[3] = Line(intPoints[2],intPoints[3]);

	blPoints[0] = Point(180, 380);
	blPoints[1] = Point(1, 475);
	blPoints[2] = Point(300, 520);
	blPoints[3] = Point(340, 470);
	blLines[0] = Line(blPoints[0],blPoints[1]);
	blLines[1] = Line(blPoints[0],blPoints[2]);
	blLines[2] = Line(blPoints[1],blPoints[3]);
	blLines[3] = Line(blPoints[2],blPoints[3]);
	
}

bool inIntersection(Line lines[], Line boundLines[]) {
	bool isCarPointIn[4] = {false};
	int countTrues = 0;
	bool lIntersections[4] = {false};

	for (int i = 0; i < 4; i++) {
			
			for (int j = 0; j < 4; j++) {
				lIntersections[j] = doLinesIntersect(lines[i], boundLines[j]);
			}
			int countIntersects = 0;
			for (int j = 0; j < 4; j++) {
				if (lIntersections[j]) { 
					countIntersects++;
					lIntersections[j] = false;
				}
			}
			if (countIntersects == 1) isCarPointIn[i] = true;
			//printf("count intersects %d\n", countIntersects);
		}
		
		countTrues = 0;
		for (int i = 0; i < 4; i++) {
			if (isCarPointIn[i]) {
				countTrues++;
				isCarPointIn[i] = false;
			}
		}

		printf("count trues %d\n", countTrues);
		if (countTrues >= 2) {
			printf("car in intersection\n");
			return true;
		}
		return false;
}




