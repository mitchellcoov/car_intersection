#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace cv;

int main(void) {

	Mat frame, grey, edges, dst, cdst;


	VideoCapture capture(1);


	if (!capture.isOpened()) {
		std::cout << "error opening camera" << std::endl;
	}

	while (true) {

		capture >> frame;
		if (frame.empty()) {
			std::cout << "frame is empty" << std::endl;
		}
/*
		cvtColor(frame, grey, CV_BGR2GRAY);
		blur(grey, edges, Size(3,3));
		Canny(edges, edges, 1, 100, 3);

		dst = Scalar::all(0);
		frame.copyTo(dst, edges);
*/
		Canny(frame, edges, 50, 200, 3);
		cvtColor(edges, cdst, CV_GRAY2BGR);

		vector<Vec2f> lines;
		HoughLines(edges, lines, 1, CV_PI/180, 100, 0, 0 );
		for( size_t i = 0; i < lines.size(); i++ )
		{
		  float rho = lines[i][0], theta = lines[i][1];
		  Point pt1, pt2;
		  double a = cos(theta), b = sin(theta);
		  double x0 = a*rho, y0 = b*rho;
		  pt1.x = cvRound(x0 + 1000*(-b));
		  pt1.y = cvRound(y0 + 1000*(a));
		  pt2.x = cvRound(x0 - 1000*(-b));
		  pt2.y = cvRound(y0 - 1000*(a));
		  line( cdst, pt1, pt2, Scalar(0,0,255), 1, 4);
		}

		//imshow("edges", dst);
		imshow("capture", frame);
		imshow("lines", cdst);



		char c = (char)waitKey(25);
		if (c == 27) break;
	}

	capture.release();
	destroyAllWindows();

	return 0;
}
